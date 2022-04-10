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
	QFutureWatcher<void> *future = NULL;
	FlashTask *work = NULL;

  public:
	explicit SingleBurnWindow(QWidget *parent = nullptr);
	~SingleBurnWindow();

	void setLibrary(class BurnLibrary *lib);

  signals:
	class FlashTask *startBurn(const QString &sysImg);

  private slots:
	void setEnabled(bool enabled);
	void resumeState();
	void resetProgressState();
	void setProgressText(const QString &tip);
	void on_btnStartBurn_clicked();
	void handleDeviceStateChange(const struct kburnDeviceNode *dev);
	void handleSerialPortList(const QMap<QString, QString> &list);

  private:
	Ui::SingleBurnWindow *ui;
};
