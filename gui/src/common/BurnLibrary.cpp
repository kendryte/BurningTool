#include "BurnLibrary.h"
#include "BuringProcess.h"
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

BurnLibrary::BurnLibrary(KBCTX context) : context(context) {}

void BurnLibrary::start() {
	previousColors = kburnSetColors((kburnDebugColors){
		.red = {	.prefix = "<span style=\"color: red\">", .postfix = "</span>"},
		.green = {  .prefix = "<span style=\"color: lime\">", .postfix = "</span>"},
		.yellow = {.prefix = "<span style=\"color: yellow\">", .postfix = "</span>"},
		.grey = {	 .prefix = "<span style=\"color: grey\">", .postfix = "</span>"},
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

void BurnLibrary::startBurn(const QString &serialPath) {
	int exists = knownSerialPorts.indexOf(serialPath);
	FlashTask *task = new FlashTask(context);
	if (exists < 0) {
		task->setSystemImageFile(serialPath);
	} else {
		task->setSystemImageFile(list.list[exists].path);
	}

	QThreadPool::globalInstance()->start(task);
}

void BurnLibrary::handleDebugLog(kburnLogType level, const char *cstr) {
	auto line = QString::fromUtf8(cstr);
	std::cerr << cstr << std::endl;
	emit onDebugLog(line);
}

bool BurnLibrary::handleConnectSerial(const kburnDeviceNode *dev) {
	reloadList();
	emit onConnectSerial(dev);
	return false;
}
bool BurnLibrary::handleConnectUsb(const kburnDeviceNode *dev) {
	emit onConnectUsb(dev);
	return true;
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
		QTextStream(&r) << list.list[i].path << " - [" << list.list[i].usbIdVendor << ":" << list.list[i].usbIdProduct << "] "
#if !WIN32
						<< list.list[i].usbDriver
#endif
			;
		knownSerialPorts.append(r);
	}

	emit onSerialPortList(knownSerialPorts);
}

void BurnLibrary::handleDeviceRemove(const kburnDeviceNode *dev) {
	reloadList();

	if (dev && dev->serial->init) {
		for (auto itr = nodes.constBegin(); itr != nodes.constEnd(); itr++) {
			if (QString::fromLatin1((*itr)->serial->deviceInfo.path) == QString::fromLatin1(dev->serial->deviceInfo.path)) {
				qDebug() << "known device disconnect" << QChar::LineFeed;
				emit onDeviceRemove(*itr);
				return;
			}
		}
	}
}
void BurnLibrary::handleHandleSerial(kburnDeviceNode *dev) {
	if (!dev) {
		return;
	}

	qDebug() << "new device handle" << QChar::LineFeed;

	nodes.append(dev);

	emit onHandleSerial(dev);

	if (dev->error->code != KBurnNoErr) {
		return;
	}

	if (!kburnSerialIspSetBaudrateHigh(dev->serial)) {
		emit onHandleSerial(dev);
	}

	if (!kburnSerialIspSwitchUsbMode(dev->serial, BurnLibrary::__handle_progress, NULL)) {
		emit onHandleSerial(dev);
	}
}
void BurnLibrary::handleHandleUsb(kburnDeviceNode *dev) { emit onHandleUsb(dev); }

void BurnLibrary::__handle_progress(void *self, const kburnDeviceNode *dev, size_t current, size_t length) {
	reinterpret_cast<BurnLibrary *>(self)->handleProgressChange(dev, current, length);
}
void BurnLibrary::handleProgressChange(const kburnDeviceNode *dev, size_t current, size_t length) {
	// todo
	qDebug() << current << length << '\n';
}
