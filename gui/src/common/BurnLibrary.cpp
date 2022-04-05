#include "BurnLibrary.h"
#include <canaan-burn/canaan-burn.h>

BurnLibrary::~BurnLibrary() {
	kburnSetColors(previousColors);

	kburnSetLogCallback(previousOnDebugLog.handler, previousOnDebugLog.context);
	kburnOnSerialConnect(context, previousOnConnectSerial.handler, previousOnConnectSerial.context);
	kburnOnSerialConfirm(context, previousOnHandleSerial.handler, previousOnHandleSerial.context);
	kburnOnUsbConnect(context, previousOnConnectUsb.handler, previousOnConnectUsb.context);
	kburnOnUsbConfirm(context, previousOnHandleUsb.handler, previousOnHandleUsb.context);
	kburnOnDeviceDisconnect(context, previousOnDeviceRemove.handler, previousOnDeviceRemove.context);
}

BurnLibrary::BurnLibrary(KBCTX context) : console(stdout), context(context) {
	previousColors = kburnSetColors((kburnDebugColors){
		.red = {	.prefix = "<span style=\"color: red\">", .postfix = "</span>"},
		.green = {  .prefix = "<span style=\"color: lime\">", .postfix = "</span>"},
		.yellow = {.prefix = "<span style=\"color: yellow\">", .postfix = "</span>"},
		.grey = { .prefix = "<span style=\"opacity: 0.5\">", .postfix = "</span>"},
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
}

void BurnLibrary::handleDebugLog(kburnLogType level, const char *cstr) {
	QString line(cstr);
	emit onDebugLog(line);
}

bool BurnLibrary::handleConnectSerial(const kburnDeviceNode *dev) {
	emit onConnectSerial(dev);
	return false;
}
bool BurnLibrary::handleConnectUsb(const kburnDeviceNode *dev) {
	emit onConnectUsb(dev);
	return false;
}
void BurnLibrary::handleDeviceRemove(const kburnDeviceNode *dev) { emit onDeviceRemove(dev); }
void BurnLibrary::handleHandleSerial(kburnDeviceNode *dev) { emit onHandleSerial(dev); }
void BurnLibrary::handleHandleUsb(kburnDeviceNode *dev) { emit onHandleUsb(dev); }
