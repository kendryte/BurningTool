#pragma once

#include "EventStack.h"
#include "MyException.h"
#include <canaan-burn/canaan-burn.h>
#include <QPromise>
#include <QRunnable>
#include <QString>

enum BurnStage {
	Starting,
	Serial,
	Usb,
};

class FlashTask : public QRunnable {
  private:
    FlashTask();

	const QString systemImage;
	const QString comPort;
	KBCTX scope;

	EventStack inputs;
	QPromise<void> output;

	KBurnException *result;

  public:
    ~FlashTask();
    FlashTask(KBCTX scope, const QString &comPort, const QString &systemImage) : scope(scope), comPort(comPort), systemImage(systemImage), inputs(2) {
        this->setAutoDelete(false);
    };
    void run();

	auto future() { return output.future(); }
	void cancel();

	const KBurnException *getResult() { return result; }

  signals:
    void onProgress(uint64_t current, uint64_t total, enum BurnStage usb_stage);

  public slots:
    void onSerialConnected(kburnDeviceNode *node) { inputs.set(0, node); };
    void onUsbConnected(kburnDeviceNode *node) { inputs.set(1, node); };
};
