#include "MainWindow.h"
#include "common/BurningProcess.h"
#include "common/BurningRequest.h"
#include "common/BurnLibrary.h"
#include "common/UpdateChecker.h"
#include "config.h"
#include "main.h"
#include "ui_MainWindow.h"
#include "widgets/SingleBurnWindow.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QList>
#include <QScrollBar>
#include <QTimer>
#include <QUrl>

#define SETTING_SPLIT_STATE "splitter-sizes"

MainWindow::~MainWindow() {
	settings.setValue(SETTING_SPLIT_STATE, ui->mainSplitter->saveState());
	delete updateChecker;
	delete ui;
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow), settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, "ui") {
	BurnLibrary::createInstance(this);

	ui->setupUi(this);

	QString getTitleVersion(void);
	setWindowTitle(windowTitle() + getTitleVersion());

	ui->mainSplitter->setStretchFactor(1, 0);
	ui->mainSplitter->setCollapsible(0, false);

	/* setting */
	qDebug() << "settings location: " << settings.fileName();
	if (settings.contains(SETTING_SPLIT_STATE)) {
		ui->mainSplitter->restoreState(settings.value(SETTING_SPLIT_STATE).toByteArray());
	} else {
		ui->mainSplitter->setSizes(QList<int>() << 100 << 0);
	}

	connect(ui->mainSplitter, &QSplitter::splitterMoved, this, &MainWindow::onResized);
	if (ui->settingsWindow->getFile().isEmpty()) {
		ui->mainSplitter->hide();
	} else {
		ui->settingsWindow->hide();
	}
	connect(ui->settingsWindow, &SettingsWindow::settingsCommited, ui->mainSplitter, &QSplitter::show);
	connect(ui->settingsWindow, &SettingsWindow::settingsCommited, ui->settingsWindow, &SettingsWindow::hide);

	connect(ui->burnControlWindow, &BurningControlWindow::showSettingRequested, ui->mainSplitter, &QSplitter::hide);
	connect(ui->burnControlWindow, &BurningControlWindow::showSettingRequested, ui->settingsWindow, &SettingsWindow::show);
	/* setting send */

	connect(&traceSetting, &SettingsBool::changed, this->ui->textLog, &LoggerWindow::setTraceLevelVisible);
	traceSetting.connectAction(ui->btnToggleLogTrace);

	connect(&logBufferSetting, &SettingsBool::changed, this, [](bool enable) { kburnSetLogBufferEnabled(enable); });
	logBufferSetting.connectAction(ui->btnDumpBuffer);

	connect(BurnLibrary::instance(), &BurnLibrary::onDebugLog, ui->textLog, &LoggerWindow::append);

	connect(ui->burnControlWindow, &BurningControlWindow::newProcessRequested, this, &MainWindow::startNewBurnJob);
	connect(ui->burnControlWindow, &BurningControlWindow::newProcessRequested, this, &MainWindow::onResized);
	connect(ui->burnControlWindow, &BurningControlWindow::newProcessRequested, ui->textLog, &LoggerWindow::clear);

	BurnLibrary::instance()->start();

	updateChecker = new UpdateChecker(ui->btnUpdate);
}

void MainWindow::onResized() {
	auto width = ui->mainContainer->geometry().width() - ui->scrollArea->verticalScrollBar()->width();
	ui->jobListContainer->setMaximumWidth(width);
	ui->jobListContainer->setMinimumWidth(width);
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

	BurnLibrary::deleteInstance();
	kburnGlobalDestroy();

	ev->accept();
}

void MainWindow::on_btnOpenWebsite_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/kendryte/BurningTool"));
}

void MainWindow::on_btnSaveLog_triggered() {
	QString selFilter("Log File (*.html)");
	QDateTime datetime = QDateTime::currentDateTime();
	QString str = QFileDialog::getSaveFileName(
		this, tr("日志保存路径"), QDir::currentPath() + "/help-" + datetime.toString("yyyyMMdd-hhmmss") + ".html",
		tr("Log File (*.html);;All files (*.*)"), &selFilter);
	if (str.isEmpty()) {
		return;
	}

	ui->textLog->copyLogFileTo(str);
}

void MainWindow::on_btnOpenRelease_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/kendryte/BurningTool/releases/tag/latest"));
}

void MainWindow::startNewBurnJob(BurningRequest *partialRequest) {
	partialRequest->systemImageFile = ui->settingsWindow->getFile();

	auto display = new SingleBurnWindow(this, partialRequest);
	ui->burnJobListView->insertWidget(ui->burnJobListView->count() - 1, display, 0, Qt::AlignTop);

	connect(display, &SingleBurnWindow::retryRequested, this, [=](BurningRequest *request) {
		QTimer::singleShot(0, this, [=] {
			// xxx
			startNewBurnJob(request);
		});

		return true;
	});

	display->show();
}

void MainWindow::on_action_triggered() {
	ui->mainSplitter->setSizes(QList<int>() << 0 << 100);
}
