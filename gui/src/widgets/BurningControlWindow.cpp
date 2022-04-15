#include "BurningControlWindow.h"
#include "common/AppGlobalSetting.h"
#include "common/BurningRequest.h"
#include "common/BurnLibrary.h"
#include "ui_BurningControlWindow.h"

struct m_portInfo : kburnSerialDeviceInfoSlice {
	QString serialized;
};

BurningControlWindow::BurningControlWindow(QWidget *parent) : QGroupBox(parent), ui(new Ui::BurningControlWindow) {
	ui->setupUi(this);
	portList = new QList<struct m_portInfo>();

	auto instance = BurnLibrary::instance();
	connect(instance, &BurnLibrary::onSerialPortList, this, &BurningControlWindow::handleSerialPortList);
	connect(instance, &BurnLibrary::jobListChanged, this, &BurningControlWindow::handleSettingsWindowButtonState);
}

BurningControlWindow::~BurningControlWindow() {
	delete ui;
	delete portList;
}

void BurningControlWindow::handleSerialPortList(const QList<kburnSerialDeviceInfoSlice> &list) {
	portList->clear();
	for (auto v : list) {
		QString r;
		r += QString::fromLatin1(v.path) + " - [" + QString::number(v.usbIdVendor, 16).leftJustified(4, '0') + ":" +
		     QString::number(v.usbIdProduct, 16).leftJustified(4, '0') + "] ";
#if WIN32
		r += QString::fromUtf8(v.title) + " (" + QString::fromUtf8(v.hwid) + ")";
#elif __linux__
		r += QString::fromLatin1(v.usbDriver);
#endif

		portList->append((m_portInfo){
			kburnSerialDeviceInfoSlice{v},
			.serialized = r,
		});
	}

	QString save(ui->inputComPortList->currentText());

	bool custom = true;
	if (save.isEmpty()) {
		custom = false;
	} else {
		for (int i = 0; i < ui->inputComPortList->count(); i++) {
			if (ui->inputComPortList->itemText(i) == save) {
				custom = false;
				break;
			}
		}
	}

	ui->inputComPortList->clear();
	for (auto info : *portList) {
		ui->inputComPortList->addItem(info.serialized);
		if (!custom) {
			if (info.serialized == save) {
				ui->inputComPortList->setCurrentIndex(ui->inputComPortList->count() - 1);
			}
		}
	}
	if (custom) {
		ui->inputComPortList->setEditText(save);
	}

	if (autoBurningEnabled) {
		for (auto comPort : *portList) {
			if (comPort.usbIdVendor != GlobalSetting::watchVid.getValue() || comPort.usbIdProduct != GlobalSetting::watchPid.getValue()) {
				continue;
			}
			auto request = new K510BurningRequest();
			request->comPort = comPort.path;
			request->isAutoCreate = true;
			if (!BurnLibrary::instance()->hasBurning(request)) {
				emit newProcessRequested(request);
			}
		}
	}
}

void BurningControlWindow::on_buttonStartAuto_clicked(bool checked) {
	autoBurningEnabled = checked;
	handleSettingsWindowButtonState();
}

void BurningControlWindow::handleSettingsWindowButtonState() {
	if (autoBurningEnabled) {
		ui->btnOpenSettings->setEnabled(false);
	} else if (BurnLibrary::instance()->getBurningJobCount() > 0) {
		ui->btnOpenSettings->setEnabled(false);
	} else {
		ui->btnOpenSettings->setEnabled(true);
	}
}

void BurningControlWindow::on_btnStartBurn_clicked() {
	QString comPort = ui->inputComPortList->currentText();
	if (comPort.isEmpty()) {
		return;
	}

	for (auto info : *portList) {
		if (info.serialized == comPort) {
			comPort = info.path;
			break;
		}
	}

	auto request = new K510BurningRequest();

	request->comPort = comPort;
	request->isAutoCreate = false;

	emit newProcessRequested(request);
}
