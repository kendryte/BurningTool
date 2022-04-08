#pragma once

#include "main.h"
#include <QObject>
#include <QString>

class BurnLibrary : public QObject {
	Q_OBJECT

	KBCTX context;

	on_device_connect_t previousOnConnectUsb;
	on_device_connect_t previousOnConnectSerial;
	on_device_remove_t previousOnDeviceRemove;
	on_device_handle_t previousOnHandleSerial;
	on_device_handle_t previousOnHandleUsb;
	on_debug_log_t previousOnDebugLog;
	kburnDebugColors previousColors;

	kburnSerialDeviceList list = {0, NULL};

	QStringList knownSerialPorts;
	QList<kburnDeviceNode *> nodes;

  public:
	BurnLibrary(KBCTX context);
	~BurnLibrary();

	void reloadList();
	void start();

  public slots:
	void startBurn(const QString &serialPath);

  private:
	bool handleConnectSerial(const kburnDeviceNode *dev);
	bool handleConnectUsb(const kburnDeviceNode *dev);
	void handleDeviceRemove(const kburnDeviceNode *dev);
	void handleHandleSerial(kburnDeviceNode *dev);
	void handleHandleUsb(kburnDeviceNode *dev);
	void handleDebugLog(kburnLogType type, const char *message);
	static void __handle_progress(void *ctx, const kburnDeviceNode *dev, size_t current, size_t length);

	void handleProgressChange(const kburnDeviceNode *dev, size_t current, size_t length);

  signals:
	bool onConnectSerial(const kburnDeviceNode *dev);
	bool onConnectUsb(const kburnDeviceNode *dev);
	void onDeviceRemove(const kburnDeviceNode *dev);
	void onHandleSerial(kburnDeviceNode *dev);
	void onHandleUsb(kburnDeviceNode *dev);
	void onDebugLog(QString message);
	void onSerialPortList(const QStringList &onSerialPortList);

	void onProgressChange(const kburnDeviceNode *dev, size_t current, size_t length);
};
