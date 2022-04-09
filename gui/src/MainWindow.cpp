#include "MainWindow.h"
#include "common/BurnLibrary.h"
#include "main.h"
#include "ui_MainWindow.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QList>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	ui->mainSplitter->setStretchFactor(1, 0);
	ui->mainSplitter->setCollapsible(0, false);

	QSettings settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, "ui");
	qDebug() << "settings location: " << settings.fileName();
	if (settings.contains("splitterSizes")) {
		ui->mainSplitter->restoreState(settings.value("splitterSizes").toByteArray());
	} else {
		ui->mainSplitter->setSizes(QList<int>() << 100 << 0);
	}

	ui->mainTabView->setCurrentIndex(0);

	ui->mainTabView->removeTab(3);
	ui->mainTabView->removeTab(2);

	kburn_err_t err = kburnCreate(&context);
	if (err != KBurnNoErr) {
		fatalAlert(err);
	}

	library = new BurnLibrary(context);
	QObject::connect(library, &BurnLibrary::onDebugLog, this->ui->textLog, &LoggerWindow::append);

	ui->manualBurnWindow->setLibrary(library);
	connect(ui->settingsWindow, &SettingsWindow::settingsChanged, this, &MainWindow::updateSettingStatus);
}

void MainWindow::updateSettingStatus() {
	QFile file(ui->settingsWindow->getFile());
	if (file.exists()) {
		if (!ui->mainTabView->isTabEnabled(1)) {
			for (auto i = 1; i < ui->mainTabView->count(); i++) {
				ui->mainTabView->setTabEnabled(i, true);
			}
		}
	} else {
		ui->mainTabView->setCurrentIndex(0);
		for (auto i = 1; i < ui->mainTabView->count(); i++) {
			ui->mainTabView->setTabEnabled(i, false);
		}
	}
}

void MainWindow::showEvent(QShowEvent *event) {
	QMainWindow::showEvent(event);
	library->start();
}

MainWindow::~MainWindow() {
	QSettings settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, "ui");
	settings.setValue("splitterSizes", ui->mainSplitter->saveState());

	delete ui;
}

void MainWindow::on_btnOpenWebsite_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/kendryte/BurningTool"));
}

void MainWindow::closeEvent(QCloseEvent *ev) {
	ui->mainTabView->setDisabled(true);
	delete library;
	kburnGlobalDestroy();
	ev->accept();
}

void MainWindow::on_btnSaveLog_triggered() {
	QString selFilter("Log File (*.log)");
	QString str =
		QFileDialog::getSaveFileName(this, tr("日志保存路径"), QDir::currentPath() + "help.log", tr("Log File (*.log);;All files (*.*)"), &selFilter);
	if (str.isEmpty()) {
		return;
	}

	// ui->textLog->toHtml()
}
