#include "MainWindow.h"
#include "common/BurnLibrary.h"
#include "config.h"
#include "main.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QList>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	BurnLibrary::createInstance(this);

	ui->setupUi(this);

	QString getTitleVersion(void);
	setWindowTitle(windowTitle() + getTitleVersion());

	ui->mainSplitter->setStretchFactor(1, 0);
	ui->mainSplitter->setCollapsible(0, false);

	QSettings settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, "ui");
	qDebug() << "settings location: " << settings.fileName();
	if (settings.contains("splitterSizes")) {
		ui->mainSplitter->restoreState(settings.value("splitterSizes").toByteArray());
	} else {
		ui->mainSplitter->setSizes(QList<int>() << 100 << 0);
	}

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
	ui->manualBurnWindow->setConfigureStata(disable);
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
	QString selFilter("Log File (*.log)");
	QString str = QFileDialog::getSaveFileName(
		this, tr("日志保存路径"), QDir::currentPath() + "/help.log", tr("Log File (*.log);;All files (*.*)"), &selFilter);
	if (str.isEmpty()) {
		return;
	}

	// ui->textLog->toHtml()
}
