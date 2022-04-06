#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#define SETTING_SYSTEM_IMAGE_PATH "sys-image-path"

SettingsWindow::SettingsWindow(QWidget *parent)
	: QWidget(parent), ui(new Ui::SettingsWindow), settings(QSettings::Scope::UserScope, "kendryte", "burning") {
	ui->setupUi(this);

	if (settings.contains(SETTING_SYSTEM_IMAGE_PATH)) {
		ui->inputSysImage->setText(settings.value(SETTING_SYSTEM_IMAGE_PATH).toString());
		loadSysImage();
	}
}

SettingsWindow::~SettingsWindow() { delete ui; }

#include <qfiledialog.h>
void SettingsWindow::on_btnSelectImage_clicked() {
	auto str = QFileDialog::getOpenFileName(this, tr("打开系统镜像"), tr(""));
	if (str.isEmpty())
		return;
	ui->inputSysImage->setText(str);

	loadSysImage();
}

void SettingsWindow::loadSysImage() {
	QString fPath = ui->inputSysImage->text();

	if (fd.fileName() == fPath)
		return;

	fd.setFileName(fPath);

	if (fd.exists()) {
		QString info;
		info += tr("file size: ") + QString::number(fd.size()) + tr(" bytes");
		ui->txtImageInfo->setText(info);
		ui->txtImageInfo->setStyleSheet("");

		settings.setValue(SETTING_SYSTEM_IMAGE_PATH, fPath);
	} else {
		ui->txtImageInfo->setText(tr("file not found"));
		ui->txtImageInfo->setStyleSheet("QLabel { color : red; }");
	}

	emit sysImageSelected(fd);
}
