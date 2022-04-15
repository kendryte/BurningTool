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
	QMap<QString, QString> portList;

  public:
	explicit BurningControlWindow(QWidget *parent = nullptr);
	~BurningControlWindow();

  signals:
	void newProcessRequested(class BurningRequest *partialRequest);
	void showSettingRequested();

  private slots:
	void on_btnStartBurn_clicked();
	void handleSerialPortList(const QMap<QString, QString> &list);
	void on_btnOpenSettings_clicked() { emit showSettingRequested(); }
	void on_buttonStartAuto_clicked(bool checked);
};

#endif // BURINGCONTROL_H
