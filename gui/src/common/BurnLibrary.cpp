#include "BurnLibrary.h"
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
	_pool->setMaxThreadCount(30);
}

BurnLibrary *BurnLibrary::_instance = NULL;

BurnLibrary *BurnLibrary::instance() {
	Q_ASSERT(BurnLibrary::_instance != NULL);
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
		QString r;
		r += QString::fromLatin1(list.list[i].path) + " - [" + QString::number(list.list[i].usbIdVendor, 16).leftJustified(4, '0') + ":" +
			 QString::number(list.list[i].usbIdProduct, 16).leftJustified(4, '0') + "] ";
#if WIN32
		r += QString::fromUtf8(list.list[i].title) + " (" + QString::fromUtf8(list.list[i].hwid) + ")";
#elif __linux__
		r += QString::fromLatin1(list.list[i].usbDriver);
#endif

		knownSerialPorts[r] = QString::fromLatin1(list.list[i].path);
	}

	emit onSerialPortList(knownSerialPorts);
}

BurningProcess *BurnLibrary::prepareBurning(const BurningRequest *request) {
	BurningProcess *work = BurningRequest::reqeustFactory(ctx, request);

	for (auto v : jobs) {
		if (work->getIdentity() == v->getIdentity()) {
			delete work;
			return NULL;
		}
	}

	jobs.append(work);

	return work;
}

void BurnLibrary::executeBurning(BurningProcess *work) {
	Q_ASSERT(jobs.contains(work));
	work->schedule();
}

bool BurnLibrary::deleteBurning(BurningProcess *task) {
	bool found = jobs.removeOne(task);

	if (!found) {
		qErrnoWarning("wired state: work \"%.10s\" not in registry", task->getTitle());
		return false;
	}

	task->cancel();
	task->deleteLater();
	return true;
}

bool BurnLibrary::handleConnectSerial(kburnDeviceNode *dev) {
	for (auto p : jobs) {
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
		p->pollingDevice(dev, Disconnected);
	}
}

void BurnLibrary::handleHandleSerial(kburnDeviceNode *dev) {
	for (auto p : jobs) {
		if (p->pollingDevice(dev, SerialReady)) {
			qDebug() << "wanted serial device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
			return;
		}
	}

	qDebug() << "unknown serial device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
}
void BurnLibrary::handleHandleUsb(kburnDeviceNode *dev) {
	for (auto p : jobs) {
		if (p->pollingDevice(dev, UsbReady)) {
			qDebug() << "wanted USB device handle: " << QString::number(dev->usb->deviceInfo.idVendor, 16) << ':'
					 << QString::number(dev->usb->deviceInfo.idProduct, 16) << QChar::LineFeed;
			return;
		}
	}

	qDebug() << "unknown USB device handle: " << QString::number(dev->usb->deviceInfo.idVendor, 16) << ':'
			 << QString::number(dev->usb->deviceInfo.idProduct, 16) << QChar::LineFeed;
}
