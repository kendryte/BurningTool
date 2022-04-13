#pragma once

#include "common/BurningProcess.h"
#include <canaan-burn/canaan-burn.h>
#include <QFutureWatcher>
#include <QWidget>

namespace Ui {
class SingleBurnWindow;
}

class SingleBurnWindow : public QWidget {
	Q_OBJECT

	Ui::SingleBurnWindow *ui;
	QFutureWatcher<void> *future = NULL;
	FlashTask *work = NULL;
	bool shown = false;

	void resumeState();

  public:
    explicit SingleBurnWindow(QWidget *parent = nullptr);
    ~SingleBurnWindow();

	void showEvent(QShowEvent *event);
	void setConfigureStata(bool incomplete);

  signals:
    class FlashTask *startedBurn(const QString &sysImg);
    class FlashTask *completedBurn(bool successful);

  private slots:
    void setEnabled(bool enabled);
    void resetProgressState();
    void setCompleteState();
    void setErrorState();
    void setProgressText(const QString &tip);
    void on_btnStartBurn_clicked();
    void handleDeviceStateChange(const struct kburnDeviceNode *dev);
    void handleSerialPortList(const QMap<QString, QString> &list);
};
