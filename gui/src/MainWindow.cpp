#include "MainWindow.h"
#include "common/BurnLibrary.h"
#include "common/UpdateChecker.h"
#include "config.h"
#include "main.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QList>
#include <QUrl>

#define SETTING_SPLIT_STATE "splitter-sizes"
#define SETTING_LOG_TRACE "log-trace"
#define SETTING_LOG_BUFFER "log-buffer"

MainWindow::~MainWindow() {
	delete updateChecker;
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow), settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, "ui") {
	BurnLibrary::createInstance(this);

	ui->setupUi(this);

	updateChecker = new UpdateChecker(ui->btnUpdate);

	QString getTitleVersion(void);
	setWindowTitle(windowTitle() + getTitleVersion());

	ui->mainSplitter->setStretchFactor(1, 0);
	ui->mainSplitter->setCollapsible(0, false);

	qDebug() << "settings location: " << settings.fileName();
	if (settings.contains(SETTING_SPLIT_STATE)) {
		ui->mainSplitter->restoreState(settings.value(SETTING_SPLIT_STATE).toByteArray());
	} else {
		ui->mainSplitter->setSizes(QList<int>() << 100 << 0);
	}

	bool shouldShowTraceLog = settings.value(SETTING_LOG_TRACE, IS_DEBUG).toBool();
	connect(ui->btnToggleLogTrace, &QAction::toggled, this->ui->textLog, &LoggerWindow::setTraceLevelVisible);
	connect(ui->btnToggleLogTrace, &QAction::toggled, this, [&](bool visible) { settings.setValue(SETTING_SPLIT_STATE, visible); });
	ui->btnToggleLogTrace->setChecked(shouldShowTraceLog);
	this->ui->textLog->setTraceLevelVisible(shouldShowTraceLog);

	bool shouldShowBufferDump = settings.value(SETTING_LOG_BUFFER, false).toBool();
	ui->btnDumpBuffer->setChecked(shouldShowBufferDump);

	auto library = BurnLibrary::instance();
	QObject::connect(library, &BurnLibrary::onDebugLog, this->ui->textLog, &LoggerWindow::append);

	connect(ui->settingsWindow, &SettingsWindow::settingsChanged, this, &MainWindow::updateSettingStatus);
	connect(ui->settingsWindow, &SettingsWindow::settingsUnsaved, this, &MainWindow::disableOtherActions);
	connect(ui->manualBurnWindow, &SingleBurnWindow::startedBurn, this->ui->textLog, &LoggerWindow::clear);
	connect(ui->manualBurnWindow, &SingleBurnWindow::startedBurn, this, [=] { ui->settingsWindow->setDisabled(true); });
	connect(ui->manualBurnWindow, &SingleBurnWindow::completedBurn, this, [=] { ui->settingsWindow->setDisabled(false); });

	updateSettingStatus();
}

void MainWindow::disableOtherActions(bool disable) {
	ui->manualBurnWindow->setConfigureState(disable);
}

void MainWindow::updateSettingStatus() {
	QFile file(ui->settingsWindow->getFile());
	BurnLibrary::instance()->setSystemImagePath(file.fileName());
}

void MainWindow::showEvent(QShowEvent *event) {
	QMainWindow::showEvent(event);

	if (shown)
		return;
	shown = true;

	ui->settingsWindow->showEvent(event);
	ui->manualBurnWindow->showEvent(event);

	BurnLibrary::instance()->start();
}

void MainWindow::on_btnOpenWebsite_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/kendryte/BurningTool"));
}

void MainWindow::closeEvent(QCloseEvent *ev) {
	if (closing) {
		QApplication::exit(0);
		return;
	}

	closing = true;
	saveWindowSize();
	setDisabled(true);

	QSettings settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, "ui");
	settings.setValue("splitterSizes", ui->mainSplitter->saveState());

	delete BurnLibrary::instance();
	kburnGlobalDestroy();

	ev->accept();
}

void MainWindow::on_btnSaveLog_triggered() {
	QString selFilter("Log File (*.html)");
	QString str = QFileDialog::getSaveFileName(
		this, tr("日志保存路径"), QDir::currentPath() + "/help.html", tr("Log File (*.html);;All files (*.*)"), &selFilter);
	if (str.isEmpty()) {
		return;
	}
	// TODO: flush and copy log file
}

void MainWindow::on_btnDumpBuffer_toggled(bool enable) {
	kburnSetLogBufferEnabled(enable);
	settings.setValue(SETTING_LOG_BUFFER, enable);
}

void MainWindow::on_btnOpenRelease_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/kendryte/BurningTool/releases/tag/latest"));
}
