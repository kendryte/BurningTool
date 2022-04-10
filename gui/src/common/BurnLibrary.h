#pragma once

#include "main.h"
#include <QMap>
#include <QObject>
#include <QString>

class BurnLibrary : public QObject {
	Q_OBJECT

	KBCTX context;

	QString imagePath;

	on_device_connect_t previousOnConnectUsb;
	on_device_connect_t previousOnConnectSerial;
	on_device_remove_t previousOnDeviceRemove;
	on_device_handle_t previousOnHandleSerial;
	on_device_handle_t previousOnHandleUsb;
	on_debug_log_t previousOnDebugLog;
	kburnDebugColors previousColors;

	kburnSerialDeviceList list = {0, NULL};

	QMap<QString, QString> knownSerialPorts;

	QMap<QString, class FlashTask *> runningFlash;

  public:
	BurnLibrary(KBCTX context);
	~BurnLibrary();

	void reloadList();
	void start();

  public slots:
	class FlashTask *startBurn(const QString &serialPath);
	void setSystemImagePath(const QString &imagePath) { this->imagePath = imagePath; }

  private:
	bool handleConnectSerial(const kburnDeviceNode *dev);
	bool handleConnectUsb(const kburnDeviceNode *dev);
	void handleDeviceRemove(const kburnDeviceNode *dev);
	void handleHandleSerial(kburnDeviceNode *dev);
	void handleHandleUsb(kburnDeviceNode *dev);
	void handleDebugLog(kburnLogType type, const char *message);

  signals:
	void onDebugLog(QString message);
	void onSerialPortList(const QMap<QString, QString> &portList);
};
