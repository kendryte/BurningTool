#pragma once

#include "common/logger.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT

  public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

  private slots:
	void on_btnSelectImage_clicked();
	void on_inputSysImage_returnPressed();
	void on_btnOpenWebsite_triggered();
	bool handleDeviceConnected(kburnDeviceNode *dev);

  private:
	Ui::MainWindow *ui;
	LogReceiver *logReceiver;
};
