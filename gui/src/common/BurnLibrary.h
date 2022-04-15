#pragma once

#include "main.h"
#include <QMap>
#include <QObject>
#include <QString>

class BurnLibrary : public QObject {
	Q_OBJECT

	QWidget *const parent;
	KBCTX ctx;
	static class BurnLibrary *_instance;

	on_device_connect_t previousOnConnectUsb;
	on_device_connect_t previousOnConnectSerial;
	on_device_remove_t previousOnDeviceRemove;
	on_device_handle_t previousOnHandleSerial;
	on_device_handle_t previousOnHandleUsb;
	on_device_list_change_t previousOnDeviceListChange;
	on_debug_log_t previousOnDebugLog;
	kburnDebugColors previousColors;

	kburnSerialDeviceList list = {0, NULL};
	QMap<QString, QString> knownSerialPorts;
	QList<class BurningProcess *> jobs; // TODO: need mutex

	BurnLibrary(QWidget *parent);

	void fatalAlert(kburn_err_t err);

	class QThreadPool *_pool;

  public:
    ~BurnLibrary();

	void reloadList();
	void start();

	static void createInstance(QWidget *parent);
	static KBCTX context();
	static BurnLibrary *instance();

	class BurningProcess *prepareBurning(const class BurningRequest *request);
	bool deleteBurning(class BurningProcess *task);
	void executeBurning(class BurningProcess *task);
	class QThreadPool *getThreadPool() {
		return _pool;
	}

	enum DeviceEvent {
		SerialAttached,
		SerialReady,
		UsbAttached,
		UsbReady,
		Disconnected,
	};

  private:
    bool handleConnectSerial(kburnDeviceNode *dev);
    bool handleConnectUsb(kburnDeviceNode *dev);
    void handleDeviceRemove(kburnDeviceNode *dev);
    void handleHandleSerial(kburnDeviceNode *dev);
    void handleHandleUsb(kburnDeviceNode *dev);
    void handleDebugLog(kburnLogType type, const char *message);
    void handleDeviceListChange(bool isUsb);

  signals:
    void onDebugLog(bool isTrace, QString message);
    void onSerialPortList(const QMap<QString, QString> &portList);
};
