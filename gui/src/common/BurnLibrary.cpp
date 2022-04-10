#include "BurnLibrary.h"
#include "BurningProcess.h"
#include <canaan-burn/canaan-burn.h>
#include <iostream>
#include <QDebug>
#include <QThreadPool>

BurnLibrary::~BurnLibrary() {
	kburnSetColors(previousColors);

	kburnSetLogCallback(previousOnDebugLog.handler, previousOnDebugLog.context);
	kburnOnSerialConnect(context, previousOnConnectSerial.handler, previousOnConnectSerial.context);
	kburnOnSerialConfirm(context, previousOnHandleSerial.handler, previousOnHandleSerial.context);
	kburnOnUsbConnect(context, previousOnConnectUsb.handler, previousOnConnectUsb.context);
	kburnOnUsbConfirm(context, previousOnHandleUsb.handler, previousOnHandleUsb.context);
	kburnOnDeviceDisconnect(context, previousOnDeviceRemove.handler, previousOnDeviceRemove.context);
}

BurnLibrary::BurnLibrary(KBCTX context) : context(context) {
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
		context, [](void *self, const kburnDeviceNode *dev) { return reinterpret_cast<BurnLibrary *>(self)->handleConnectSerial(dev); }, this);

	previousOnHandleSerial = kburnOnSerialConfirm(
		context, [](void *self, kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleHandleSerial(dev); }, this);

	previousOnConnectUsb = kburnOnUsbConnect(
		context, [](void *self, const kburnDeviceNode *dev) { return reinterpret_cast<BurnLibrary *>(self)->handleConnectUsb(dev); }, this);

	previousOnHandleUsb = kburnOnUsbConfirm(
		context, [](void *self, kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleHandleUsb(dev); }, this);

	previousOnDeviceRemove = kburnOnDeviceDisconnect(
		context, [](void *self, const kburnDeviceNode *dev) { reinterpret_cast<BurnLibrary *>(self)->handleDeviceRemove(dev); }, this);

#pragma GCC diagnostic pop

	auto err = kburnStartWaitingDevices(context);
	if (err != KBurnNoErr) {
		fatalAlert(err);
	}

	reloadList();
}

void BurnLibrary::handleDebugLog(kburnLogType level, const char *cstr) {
	auto line = QString::fromUtf8(cstr);
	std::cerr << cstr << std::endl;
	emit onDebugLog(line);
}

void BurnLibrary::reloadList() {
	auto n = kburnGetSerialList(context);
	memcpy(&list, &n, sizeof(list));

	if (list.size < 0) {
		emit onDebugLog(QString("serial device list failed."));
		return;
	}

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
	FlashTask *task = new FlashTask(context, comPortPath, imagePath);
	runningFlash[comPortPath] = task;

	QThreadPool::globalInstance()->start(task);

	return task;
}

bool BurnLibrary::handleConnectSerial(const kburnDeviceNode *dev) {
	reloadList();

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
	reloadList();

	if (dev && (dev->serial->init || dev->serial->isUsbBound)) {
		auto run = runningFlash.value(QString::fromLatin1(dev->serial->deviceInfo.path));
		if (run != NULL) {
			qDebug() << "known device disconnect" << QChar::LineFeed;
			run->cancel();

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
