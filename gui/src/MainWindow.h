#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT
	KBCTX context;
	bool closing = false;

  protected:
	void showEvent(QShowEvent *ev);

  public:
	MainWindow(QWidget *parent = nullptr);

  private slots:
	void on_btnOpenWebsite_triggered();
	void on_btnSaveLog_triggered();
	void updateSettingStatus();
	void disableOtherTabs(bool disable);

  private:
	void closeEvent(QCloseEvent *ev);
	Ui::MainWindow *ui;
};
