#ifndef BURINGCONTROL_H
#define BURINGCONTROL_H

#include <QGroupBox>
#include <QMap>

namespace Ui {
class BurningControlWindow;
}

class BurningControlWindow : public QGroupBox {
	Q_OBJECT

	Ui::BurningControlWindow *ui;
	QList<struct m_portInfo> *portList;
	bool autoBurningEnabled = false;

  public:
	explicit BurningControlWindow(QWidget *parent = nullptr);
	~BurningControlWindow();

  signals:
	void newProcessRequested(class BurningRequest *partialRequest);
	void showSettingRequested();

  private slots:
	void handleSettingsWindowButtonState();
	void on_btnStartBurn_clicked();
	void handleSerialPortList(const QList<struct kburnSerialDeviceInfoSlice> &list);
	void on_btnOpenSettings_clicked() { emit showSettingRequested(); }
	void on_buttonStartAuto_clicked(bool checked);
};

#endif // BURINGCONTROL_H
