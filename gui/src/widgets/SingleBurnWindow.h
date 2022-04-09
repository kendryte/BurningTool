#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QWidget>

namespace Ui {
class SingleBurnWindow;
}

class SingleBurnWindow : public QWidget {
	Q_OBJECT

  public:
	explicit SingleBurnWindow(QWidget *parent = nullptr);
	~SingleBurnWindow();

	void setLibrary(class BurnLibrary *lib);

  signals:
	void startBurn(const QString &sysImg);

  private slots:
	void on_btnStartBurn_clicked();
	void handleDeviceStateChange(struct kburnDeviceNode *dev);
	void handleSerialPortList(const QStringList &list);

  private:
	Ui::SingleBurnWindow *ui;
};
