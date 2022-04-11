#include "SettingsWindow.h"
#include "../common/BurnLibrary.h"
#include "ui_SettingsWindow.h"
#include <canaan-burn/canaan-burn.h>
#include <QDir>
#include <QFileDialog>
#include <QPushButton>

#define SETTING_SYSTEM_IMAGE_PATH "sys-image-path"
#define SETTING_BAUD_INIT "baudrate-init"
#define SETTING_BAUD_HIGH "baudrate-transport"
#define SETTING_BYTE_SIZE "byte-size"
#define SETTING_STOP_BITS "stop-bits"
#define SETTING_PARITY "parity"
#define SETTING_SERIAL_READ_TIMEOUT "serial-read-timeout"
#define SETTING_SERIAL_WRITE_TIMEOUT "serial-write-timeout"
#define SETTING_SERIAL_RETRY "serial-write-retry"

static const QList<uint32_t> baudrates = {9600,    115200,  230400,  460800,  576000,  921600,  1000000,
                                          1152000, 1500000, 2000000, 2500000, 3000000, 3500000, 4000000};

static const QMap<uint8_t, enum KBurnSerialConfigByteSize> bsMap = {
	{8, KBurnSerialConfigByteSize_8},
	{7, KBurnSerialConfigByteSize_7},
	{6, KBurnSerialConfigByteSize_6},
	{5, KBurnSerialConfigByteSize_5},
};

static const QMap<QString, enum KBurnSerialConfigParity> parMap = {
	{"None",  KBurnSerialConfigParityNone },
    {"Odd",   KBurnSerialConfigParityOdd  },
    {"Even",  KBurnSerialConfigParityEven },
	{"Mark",  KBurnSerialConfigParityMark },
    {"Space", KBurnSerialConfigParitySpace},
};

static const QMap<float, enum KBurnSerialConfigStopBits> sbMap = {
	{1,   KBurnSerialConfigStopBitsOne    },
	{1.5, KBurnSerialConfigStopBitsOneHalf},
	{2,   KBurnSerialConfigStopBitsTwo    },
};

SettingsWindow::SettingsWindow(QWidget *parent)
	: QWidget(parent), ui(new Ui::SettingsWindow), settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, "burning") {
	ui->setupUi(this);
}

SettingsWindow::~SettingsWindow() {
	delete ui;
}

void SettingsWindow::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);

	if (shown)
		return;
	shown = true;

	reloadSettings();
}

void SettingsWindow::on_btnSelectImage_clicked() {
	auto str = QFileDialog::getOpenFileName(this, tr("打开系统镜像"), ui->inputSysImage->text(), tr(""), nullptr, QFileDialog::ReadOnly);
	if (str.isEmpty()) {
		return;
	}
	ui->inputSysImage->setText(str);

	checkSysImage();
}

bool SettingsWindow::checkSysImage() {
	QString fPath = ui->inputSysImage->text();

	if (fd.fileName() == fPath) {
		return !fd.fileName().isEmpty();
	}

	fd.setFileName(fPath);
	if (fd.exists()) {
		QString info = tr("file size: ");

		QLocale locale = this->locale();
		info += locale.formattedDataSize(fd.size());

		ui->txtImageInfo->setText(info);
		ui->txtImageInfo->setStyleSheet("");

		inputChanged();
		return true;
	} else {
		fd.setFileName("");
		ui->txtImageInfo->setText(tr("file not found"));
		ui->txtImageInfo->setStyleSheet("QLabel { color : red; }");

		inputChanged();
		return false;
	}
}

void SettingsWindow::on_inputSysImage_editingFinished() {
	checkSysImage();
}

void SettingsWindow::on_actionBar_clicked(QAbstractButton *button) {
	if (button == (QAbstractButton *)ui->actionBar->button(QDialogButtonBox::RestoreDefaults)) {
		restoreDefaults();
	} else if (button == (QAbstractButton *)ui->actionBar->button(QDialogButtonBox::Save)) {
		acceptSave();
	} else if (button == (QAbstractButton *)ui->actionBar->button(QDialogButtonBox::Discard)) {
		reloadSettings();
	}
}

void SettingsWindow::inputChanged() {
	ui->actionBar->button(QDialogButtonBox::Save)->setEnabled(true);
	ui->actionBar->button(QDialogButtonBox::Discard)->setEnabled(true);
	emit settingsUnsaved(true);
}

void SettingsWindow::checkAndSetInt(const QString &setting, const QString &input) {
	bool ok = false;
	uint32_t value = input.toInt(&ok);
	if (ok) {
		settings.setValue(setting, value);
	}
}

template <typename keyT, typename valueT>
void SettingsWindow::checkAndSetMap(const QMap<keyT, valueT> &map, const char *config, keyT current) {
	if (map.contains(current)) {
		settings.setValue(config, current);
	}
}

void SettingsWindow::acceptSave() {
	QString fPath = fd.fileName();
	if (!fPath.isEmpty()) {
		QDir d(QCoreApplication::applicationDirPath());
		QString relPath = d.relativeFilePath(fPath);

		if (d.isRelativePath(relPath)) {
			settings.setValue(SETTING_SYSTEM_IMAGE_PATH, relPath);
		} else {
			settings.setValue(SETTING_SYSTEM_IMAGE_PATH, fPath);
		}
	}

	checkAndSetInt(SETTING_BAUD_INIT, ui->inputBaudrateInit->currentText());
	checkAndSetInt(SETTING_BAUD_HIGH, ui->inputBaudrateHigh->currentText());

	checkAndSetMap(bsMap, SETTING_BYTE_SIZE, (uint8_t)ui->inputByteSize->value());
	checkAndSetMap(parMap, SETTING_PARITY, ui->inputParity->currentText());
	checkAndSetMap(sbMap, SETTING_STOP_BITS, (float)ui->inputStopBits->value());

	reloadSettings();
}

void SettingsWindow::restoreDefaults() {
	settings.clear();
	reloadSettings();
}

void SettingsWindow::reloadSettings() {
	auto context = BurnLibrary::context();

	QDir appDir(QCoreApplication::applicationDirPath());
	if (settings.contains(SETTING_SYSTEM_IMAGE_PATH)) {
		QString saved = settings.value(SETTING_SYSTEM_IMAGE_PATH).toString();
		if (QDir(saved).isRelative()) {
			saved = QDir::cleanPath(appDir.absoluteFilePath(saved));
		}
		ui->inputSysImage->setText(saved);
		checkSysImage();
	}

	ui->inputBaudrateHigh->clear();
	ui->inputBaudrateInit->clear();
	for (auto v : baudrates) {
		ui->inputBaudrateHigh->addItem(QString::number(v), v);
		ui->inputBaudrateInit->addItem(QString::number(v), v);
	}
	{
		auto v = settings.value(SETTING_BAUD_HIGH, 921600).toUInt();
		ui->inputBaudrateHigh->setCurrentText(QString::number(v));
		kburnSetSerialBaudrate(context, v);
	}
	{
		auto v = settings.value(SETTING_BAUD_INIT, 115200).toUInt();
		ui->inputBaudrateInit->setCurrentText(QString::number(v));
		kburnSetSerialFastbootBaudrate(context, v);
	}

	{
		auto v = settings.value(SETTING_BYTE_SIZE, 8).toUInt();
		ui->inputByteSize->setValue(v);
		kburnSetSerialByteSize(context, bsMap.value(v));
	}

	{
		ui->inputParity->clear();
		for (auto v : parMap.keys()) {
			ui->inputParity->addItem(v);
		}

		auto v = settings.value(SETTING_PARITY, "None").toString();
		ui->inputParity->setCurrentText(v);
		kburnSetSerialParity(context, parMap.value(v));
	}

	{
		auto v = settings.value(SETTING_STOP_BITS, 2).toFloat();
		ui->inputStopBits->setValue(v);
		kburnSetSerialStopBits(context, sbMap.value(v));
	}

	{
		auto v = settings.value(SETTING_SERIAL_READ_TIMEOUT, 1000).toUInt();
		ui->inputSerialReadTimeout->setValue(v);
		kburnSetSerialReadTimeout(context, v);
	}

	{
		auto v = settings.value(SETTING_SERIAL_WRITE_TIMEOUT, 1000).toUInt();
		ui->inputSerialWriteTimeout->setValue(v);
		kburnSetSerialWriteTimeout(context, v);
	}

	{
		auto v = settings.value(SETTING_SERIAL_RETRY, 6).toUInt();
		ui->inputSerialRetry->setValue(v);
		kburnSetSerialFailRetry(context, v);
	}

	ui->actionBar->button(QDialogButtonBox::Save)->setEnabled(false);
	ui->actionBar->button(QDialogButtonBox::Discard)->setEnabled(false);

	emit settingsChanged();
	emit settingsUnsaved(false);
}
