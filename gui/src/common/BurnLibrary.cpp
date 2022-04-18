#include "BurnLibrary.h"
#include "AppGlobalSetting.h"
#include "BurningProcess.h"
#include "main.h"
#include <canaan-burn/canaan-burn.h>
#include <iostream>
#include <QDebug>
#include <QMessageBox>
#include <QThreadPool>

BurnLibrary::~BurnLibrary() {
	kburnSetColors(previousColors);

	kburnSetLogCallback(previousOnDebugLog.handler, previousOnDebugLog.context);
	kburnOnSerialConnect(ctx, previousOnConnectSerial.handler, previousOnConnectSerial.context);
	kburnOnSerialConfirm(ctx, previousOnHandleSerial.handler, previousOnHandleSerial.context);
	kburnOnUsbConnect(ctx, previousOnConnectUsb.handler, previousOnConnectUsb.context);
	kburnOnUsbConfirm(ctx, previousOnHandleUsb.handler, previousOnHandleUsb.context);
	kburnOnDeviceDisconnect(ctx, previousOnDeviceRemove.handler, previousOnDeviceRemove.context);

	_pool->waitForDone(5000);
	delete _pool;
}

BurnLibrary::BurnLibrary(QWidget *parent) : parent(parent) {
	_pool = new QThreadPool();
	_pool->setMaxThreadCount(qMax(GlobalSetting::appBurnThread.getValue(), (uint)50));
}

BurnLibrary *BurnLibrary::_instance = NULL;

BurnLibrary *BurnLibrary::instance() {
	return BurnLibrary::_instance;
}

KBCTX BurnLibrary::context() {
	Q_ASSERT(BurnLibrary::_instance != NULL);
	return BurnLibrary::_instance->ctx;
}

void BurnLibrary::createInstance(QWidget *parent) {
	Q_ASSERT(BurnLibrary::_instance == NULL);
	auto instance = new BurnLibrary(parent);

	BurnLibrary::_instance = instance;
};

void BurnLibrary::deleteInstance() {
	delete BurnLibrary::_instance;
	BurnLibrary::_instance = NULL;
}

void BurnLibrary::fatalAlert(kburn_err_t err) {
	auto e = kburnSplitErrorCode(err);
	QMessageBox msg(
		QMessageBox::Icon::Critical, ::tr("错误"),
		::tr("无法初始化读写功能:") + '\n' + ::tr("error kind: ") + QString::number(e.kind) + ", " + ::tr("code: ") + QString::number(e.code),
		QMessageBox::StandardButton::Close, parent);
	msg.exec();
	abort();
}

void BurnLibrary::start() {
	previousColors = kburnSetColors((kburnDebugColors){
		.red = {.prefix = "<span style=\"color: red\">",    .postfix = "</span>"},
		.green = {.prefix = "<span style=\"color: lime\">",   .postfix = "</span>"},
		.yellow = {.prefix = "<span style=\"color: yellow\">", .postfix = "</span>"},
		.grey = {.prefix = "<span style=\"color: grey\">",   .postfix = "</span>"},
	});

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
	previousOnDebugLog = kburnSetLogCallback(
		[](void *self, kburnLogType level, const char *cstr) { reinterpret_cast<BurnLibrary *>(self)->handleDebugLog(level, cstr); }, this);

	KBCTX context;
	kburn_err_t err = kburnCreate(&context);
	if (err != KBurnNoErr) {
		fatalAlert(err);
	}

	ctx = context;

	previousOnConnectSerial = kburnOnSerialConnect(
		ctx, [](void *self, kburnDeviceNode *dev) { return reinterpret_cast<BurnLibrary *>(self)->handleConnectSerial(dev); }, this);

	previousOnHandleSerial = kburnOnSerialConfirm(
		ctx, [](void *self, kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleHandleSerial(dev); }, this);

	previousOnConnectUsb = kburnOnUsbConnect(
		ctx, [](void *self, kburnDeviceNode *dev) { return reinterpret_cast<BurnLibrary *>(self)->handleConnectUsb(dev); }, this);

	previousOnHandleUsb = kburnOnUsbConfirm(
		ctx, [](void *self, kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleHandleUsb(dev); }, this);

	previousOnDeviceRemove = kburnOnDeviceDisconnect(
		ctx, [](void *self, kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleDeviceRemove(dev); }, this);

	previousOnDeviceListChange = kburnOnDeviceListChange(
		ctx, [](void *self, bool isUsb) { reinterpret_cast<BurnLibrary *>(self)->handleDeviceListChange(isUsb); }, this);

#pragma GCC diagnostic pop

	err = kburnStartWaitingDevices(ctx);
	if (err != KBurnNoErr) {
		fatalAlert(err);
	}

	handleDeviceListChange(false);
}

void BurnLibrary::handleDebugLog(kburnLogType level, const char *cstr) {
	auto line = QString::fromUtf8(cstr);
	std::cerr << cstr << std::endl;
	emit onDebugLog(level == KBURN_LOG_TRACE, line);
}

void BurnLibrary::handleDeviceListChange(bool isUsb) {
	if (!isUsb) {
		reloadList();
	}
}

void BurnLibrary::reloadList() {
	auto n = kburnGetSerialList(ctx);
	memcpy(&list, &n, sizeof(list));

	if (list.size < 0) {
		emit onDebugLog(false, QString("serial device list failed."));
		return;
	}

	emit onDebugLog(false, QString("kburnGetSerialList: ") + QString::number(list.size));

	knownSerialPorts.clear();
	for (auto i = 0; i < list.size; i++) {
		knownSerialPorts.append(list.list[i]);
	}

	emit onSerialPortList(knownSerialPorts);
}

BurningProcess *BurnLibrary::prepareBurning(const BurningRequest *request) {
	if (hasBurning(request)) {
		return NULL;
	}
	BurningProcess *work = BurningRequest::reqeustFactory(ctx, request);

	jobs[request->getIdentity()] = work;

	return work;
}

void BurnLibrary::executeBurning(BurningProcess *work) {
	Q_ASSERT(jobs.values().contains(work));
	work->schedule();
	emit jobListChanged();
}

bool BurnLibrary::hasBurning(const BurningRequest *request) {
	return jobs.contains(request->getIdentity());
}

bool BurnLibrary::deleteBurning(BurningProcess *task) {
	bool found = false;

	for (auto it = jobs.begin(); it != jobs.end(); it++) {
		if (it.value() == task) {
			jobs.erase(it);
			found = true;
			break;
		}
	}

	if (!found) {
		qErrnoWarning("wired state: work \"%.10s\" not in registry", task->getTitle().toLatin1().constData());
		return false;
	}

	emit jobListChanged();
	if (task->isStarted() && !task->isCompleted()) {
		// TODO: 有些情况需要cancel()而不是等结束
		connect(task, &BurningProcess::completed, task, &BurningProcess::deleteLater);
	} else {
		task->deleteLater();
	}
	return true;
}

static bool should_skip_job(const BurningProcess *p) {
	return !p->isStarted() || p->isCompleted() || p->isCanceled();
}

bool BurnLibrary::handleConnectSerial(kburnDeviceNode *dev) {
	for (auto p : jobs) {
		if (should_skip_job(p)) {
			continue;
		}
		if (p->pollingDevice(dev, SerialAttached)) {
			return true;
		}
	}
	return false;
}

bool BurnLibrary::handleConnectUsb(kburnDeviceNode *dev) {
	return true;
}

void BurnLibrary::handleDeviceRemove(kburnDeviceNode *dev) {
	for (auto p : jobs) {
		if (should_skip_job(p)) {
			continue;
		}
		p->pollingDevice(dev, Disconnected);
	}
}

void BurnLibrary::handleHandleSerial(kburnDeviceNode *dev) {
	for (auto p : jobs) {
		if (should_skip_job(p)) {
			continue;
		}
		if (p->pollingDevice(dev, SerialReady)) {
			qDebug() << "wanted serial device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
			return;
		}
	}

	qDebug() << "unknown serial device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
}
void BurnLibrary::handleHandleUsb(kburnDeviceNode *dev) {
	for (auto p : jobs) {
		if (should_skip_job(p)) {
			continue;
		}
		if (p->pollingDevice(dev, UsbReady)) {
			qDebug() << "wanted USB device handle: " << QString::number(dev->usb->deviceInfo.idVendor, 16) << ':'
					 << QString::number(dev->usb->deviceInfo.idProduct, 16) << QChar::LineFeed;
			return;
		}
	}

	qDebug() << "unknown USB device handle: " << QString::number(dev->usb->deviceInfo.idVendor, 16) << ':'
			 << QString::number(dev->usb->deviceInfo.idProduct, 16) << QChar::LineFeed;
}
