#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QObject>
#include <QString>
#include <QTextStream>

class BurnLibrary : public QObject {
	Q_OBJECT

	QTextStream console;
	KBCTX context;

	on_device_connect_t previousOnConnectUsb;
	on_device_connect_t previousOnConnectSerial;
	on_device_remove_t previousOnDeviceRemove;
	on_device_handle_t previousOnHandleSerial;
	on_device_handle_t previousOnHandleUsb;
	on_debug_log_t previousOnDebugLog;
	kburnDebugColors previousColors;

  public:
	BurnLibrary(KBCTX context);
	~BurnLibrary();

  private:
	bool handleConnectSerial(const kburnDeviceNode *dev);
	bool handleConnectUsb(const kburnDeviceNode *dev);
	void handleDeviceRemove(const kburnDeviceNode *dev);
	void handleHandleSerial(kburnDeviceNode *dev);
	void handleHandleUsb(kburnDeviceNode *dev);
	void handleDebugLog(kburnLogType type, const char *message);

  signals:
	bool onConnectSerial(const kburnDeviceNode *dev);
	bool onConnectUsb(const kburnDeviceNode *dev);
	void onDeviceRemove(const kburnDeviceNode *dev);
	void onHandleSerial(kburnDeviceNode *dev);
	void onHandleUsb(kburnDeviceNode *dev);
	void onDebugLog(QString message);
};
