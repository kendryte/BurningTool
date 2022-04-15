#pragma once

#include "common/SettingsHelper.h"
#include <canaan-burn/canaan-burn.h>
#include <QMainWindow>
#include <QSettings>

#define SETTING_LOG_TRACE "log-trace"
#define SETTING_LOG_BUFFER "log-buffer"

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

	SettingsBool traceSetting{"debug", SETTING_LOG_TRACE, IS_DEBUG};
	SettingsBool logBufferSetting{"debug", SETTING_LOG_BUFFER, false};

  protected:
  public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void resizeEvent(QResizeEvent *event) { onResized(); }

  private slots:
	void on_btnOpenWebsite_triggered();
	void on_btnSaveLog_triggered();
	void on_btnOpenRelease_triggered();
	void startNewBurnJob(class BurningRequest *partialRequest);
	void onResized();

  private:
	void closeEvent(QCloseEvent *ev);
	Ui::MainWindow *ui;
};
