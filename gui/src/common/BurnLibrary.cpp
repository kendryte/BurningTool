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
}

BurnLibrary::BurnLibrary(QWidget *parent) : parent(parent) {
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

	KBCTX context;
	kburn_err_t err = kburnCreate(&context);
	if (err != KBurnNoErr) {
		instance->fatalAlert(err);
	}

	instance->ctx = context;

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

	previousOnConnectSerial = kburnOnSerialConnect(
		ctx, [](void *self, const kburnDeviceNode *dev) { return reinterpret_cast<BurnLibrary *>(self)->handleConnectSerial(dev); }, this);

	previousOnHandleSerial = kburnOnSerialConfirm(
		ctx, [](void *self, kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleHandleSerial(dev); }, this);

	previousOnConnectUsb = kburnOnUsbConnect(
		ctx, [](void *self, const kburnDeviceNode *dev) { return reinterpret_cast<BurnLibrary *>(self)->handleConnectUsb(dev); }, this);

	previousOnHandleUsb = kburnOnUsbConfirm(
		ctx, [](void *self, kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleHandleUsb(dev); }, this);

	previousOnDeviceRemove = kburnOnDeviceDisconnect(
		ctx, [](void *self, const kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleDeviceRemove(dev); }, this);

	previousOnDeviceListChange = kburnOnDeviceListChange(
		ctx, [](void *self, bool isUsb) { reinterpret_cast<BurnLibrary *>(self)->handleDeviceListChange(isUsb); }, this);

#pragma GCC diagnostic pop

	auto err = kburnStartWaitingDevices(ctx);
	if (err != KBurnNoErr) {
		fatalAlert(err);
	}

	handleDeviceListChange(false);
}

void BurnLibrary::handleDebugLog(kburnLogType level, const char *cstr) {
	auto line = QString::fromUtf8(cstr);
	std::cerr << cstr << std::endl;
	emit onDebugLog(line);
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
		emit onDebugLog(QString("serial device list failed."));
		return;
	}

	emit onDebugLog(QString("kburnGetSerialList: ") + QString::number(list.size));

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

FlashTask *BurnLibrary::startBurn(const QString &selectedSerial) {
	const QString &comPortPath = knownSerialPorts.value(selectedSerial, selectedSerial);
	FlashTask *task = new FlashTask(ctx, comPortPath, imagePath);

	if (runningFlash.contains(comPortPath)) {
		runningFlash[comPortPath]->cancel();
		deleteBurnTask(runningFlash[comPortPath]);
	}

	runningFlash[comPortPath] = task;

	QThreadPool::globalInstance()->start(task);

	return task;
}

bool BurnLibrary::deleteBurnTask(FlashTask *task) {
	if (!runningFlash.values().contains(task)) {
		return false;
	}

	runningFlash.remove(task->comPort);
	delete task;

	return true;
}

bool BurnLibrary::handleConnectSerial(const kburnDeviceNode *dev) {
	auto run = runningFlash.value(QString::fromLatin1(dev->serial->deviceInfo.path));
	if (run != NULL) {
		return true;
	}

	return false;
}
bool BurnLibrary::handleConnectUsb(const kburnDeviceNode *dev) {
	return true;
}
void BurnLibrary::handleDeviceRemove(const kburnDeviceNode *dev) {
	if (dev && dev->serial) {
		auto path = QString::fromLatin1(dev->serial->deviceInfo.path);
		auto run = runningFlash.value(path);
		if (run != NULL) {
			qDebug() << "known device disconnect" << QChar::LineFeed;
			run->cancel();
			runningFlash.remove(path);

			return;
		}
	}
}
void BurnLibrary::handleHandleSerial(kburnDeviceNode *dev) {
	auto run = runningFlash.value(QString::fromLatin1(dev->serial->deviceInfo.path));
	if (run != NULL) {
		qDebug() << "wanted serial device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
		run->onSerialConnected(dev);
		return;
	}

	qDebug() << "unknown serial device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
}
void BurnLibrary::handleHandleUsb(kburnDeviceNode *dev) {
	if (dev && (dev->serial->init || dev->serial->isUsbBound)) {
		auto run = runningFlash.value(QString::fromLatin1(dev->serial->deviceInfo.path));
		if (run != NULL) {
			qDebug() << "wanted usb device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
			run->onUsbConnected(dev);
		}
	}

	qDebug() << "unknown usb device handle: " << dev->serial->deviceInfo.path << QChar::LineFeed;
}
