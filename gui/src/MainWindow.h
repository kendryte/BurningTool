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

  protected:
    void showEvent(QShowEvent *ev);

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
    void on_btnOpenWebsite_triggered();
    void on_btnSaveLog_triggered();
    void updateSettingStatus();

  private:
    void closeEvent(QCloseEvent *ev);
    Ui::MainWindow *ui;
    class BurnLibrary *library;
};
