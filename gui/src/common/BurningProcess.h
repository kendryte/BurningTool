#pragma once

#include "EventStack.h"
#include "MyException.h"
#include <canaan-burn/canaan-burn.h>
#include <QPromise>
#include <QRunnable>
#include <QString>

enum BurnStage
{
	Starting,
	Serial,
	Usb,
};

class FlashTask : public QObject, public QRunnable {
	Q_OBJECT

  private:
	FlashTask();

	size_t bytesWritten = 0;
	size_t bytesNextStage = 0;
	const QString systemImage;
	const QString comPort;
	KBCTX scope;

	EventStack inputs;
	QPromise<void> output;

	KBurnException *result = NULL;

	void setProgressValue(size_t bytes);
	static void serial_isp_progress(void *, const kburnDeviceNode *, size_t, size_t);

	bool canceled = false;
	void testCancel();

  public:
	~FlashTask();
	FlashTask(KBCTX scope, const QString &comPort, const QString &systemImage) : scope(scope), comPort(comPort), systemImage(systemImage), inputs(2) {
		this->setAutoDelete(false);
	};
	void run();

	auto future() { return output.future(); }
	void cancel();

	const KBurnException *getResult() { return result; }

	void nextStage(const QString &title, size_t bytes = 0);

  signals:
	void onDeviceChange(const kburnDeviceNode *node);
	void progressTextChanged(const QString &title);

  public slots:
	void onSerialConnected(kburnDeviceNode *node) {
		inputs.set(0, node);
		emit onDeviceChange(node);
	};
	void onUsbConnected(kburnDeviceNode *node) {
		inputs.set(1, node);
		emit onDeviceChange(node);
	};
};
