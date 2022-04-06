#pragma once

#include "common/BurnLibrary.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT
	KBCTX context;

  protected:
	void showEvent(QShowEvent *ev);

  public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

  private slots:
	void on_btnSelectImage_clicked();
	void on_inputSysImage_returnPressed();
	void on_btnOpenWebsite_triggered();
	void on_btnStartBurn_clicked();
	void handleDeviceStateChange(kburnDeviceNode *dev);
	void handleSerialPortList(const QStringList &list);

  private:
	Ui::MainWindow *ui;
	BurnLibrary *library;
};
