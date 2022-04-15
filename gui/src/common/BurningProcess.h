#pragma once

#include "BurningRequest.h"
#include "BurnLibrary.h"
#include "MyException.h"
#include <canaan-burn/canaan-burn.h>
#include <QFile>
#include <QObject>
#include <QRunnable>
#include <QString>

class BurningProcess : public QObject, public QRunnable {
	Q_OBJECT

	QFile imageFile;
	class QByteArray *buffer = NULL;
	bool _isCanceled = false;
	bool _isStarted = false;
	bool _isCompleted = false;
	KBurnException _result;

  protected:
    BurningProcess(KBCTX scope, const BurningRequest *request);

	KBCTX scope;
	class QDataStream *imageStream = NULL;

	void setResult(const KBurnException &reason);
	void throwIfCancel();

	void setStage(const QString &title, int bytesToWrite = 0);
	void setProgress(int writtenBytes);

	virtual qint64 prepare() = 0;
	virtual bool step(kburn_stor_address_t address, const QByteArray &chunk) = 0;
	virtual void cleanup(bool success){};

  public:
    const qint64 imageSize;
    ~BurningProcess();

	void run() Q_DECL_NOTHROW;
	void _run();
	void schedule();

	virtual QString getTitle() const { return "UNKNOWN JOB"; }
	virtual const QString &getDetailInfo() const = 0;
	virtual bool pollingDevice(kburnDeviceNode *node, BurnLibrary::DeviceEvent event) = 0;
	const KBurnException &getReason() { return _result; }

	bool isCanceled() { return _isCanceled; }
	bool isStarted() { return _isStarted; }
	bool isCompleted() { return _isCompleted; }

	virtual void cancel(const KBurnException reason);
	virtual void cancel();

	enum BurnStage {
		Starting,
		Serial,
		Usb,
	};
  signals:
    void deviceStateNotify();

	void stageChanged(const QString &title);
	void bytesChanged(int maximumBytes);
	void progressChanged(int writtenBytes);

	void cancelRequested();
	void completed();
	void failed(const KBurnException &reason);
};
