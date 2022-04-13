#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QMainWindow>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT
	KBCTX context;
	bool closing = false;
	bool shown = false;

	QSettings settings;
	class UpdateChecker *updateChecker;

  protected:
    void showEvent(QShowEvent *ev);

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
    void on_btnOpenWebsite_triggered();
    void on_btnSaveLog_triggered();
    void updateSettingStatus();
    void disableOtherActions(bool disable);
    void on_btnDumpBuffer_toggled(bool enable);

    void on_btnOpenRelease_triggered();

  private:
    void closeEvent(QCloseEvent *ev);
    Ui::MainWindow *ui;
};
