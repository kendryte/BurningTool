#ifndef BURINGCONTROL_H
#define BURINGCONTROL_H

#include <QGroupBox>
#include <QMap>

namespace Ui {
class BuringControlWindow;
}

class BuringControlWindow : public QGroupBox {
	Q_OBJECT

	Ui::BuringControlWindow *ui;
	QMap<QString, QString> portList;

  public:
	explicit BuringControlWindow(QWidget *parent = nullptr);
	~BuringControlWindow();

  signals:
	void newProcessRequested(class BuringRequest *partialRequest);
	void showSettingRequested();

  private slots:
	void on_btnStartBurn_clicked();
	void handleSerialPortList(const QMap<QString, QString> &list);
	void on_btnAdvance_clicked() { emit showSettingRequested(); }
	void on_buttonStartAuto_clicked(bool checked);
};

#endif // BURINGCONTROL_H
