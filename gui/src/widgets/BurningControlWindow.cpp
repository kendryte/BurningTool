#include "BurningControlWindow.h"
#include "common/BurningRequest.h"
#include "common/BurnLibrary.h"
#include "ui_BurningControlWindow.h"

BurningControlWindow::BurningControlWindow(QWidget *parent) : QGroupBox(parent), ui(new Ui::BurningControlWindow) {
	ui->setupUi(this);

	connect(BurnLibrary::instance(), &BurnLibrary::onSerialPortList, this, &BurningControlWindow::handleSerialPortList);
}

BurningControlWindow::~BurningControlWindow() {
	delete ui;
}

void BurningControlWindow::handleSerialPortList(const QMap<QString, QString> &list) {
	portList = list;

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
	auto titles = list.keys();
	for (auto title : titles) {
		ui->inputComPortList->addItem(title, list.value(title));
	}
	if (custom) {
		ui->inputComPortList->setEditText(save);
	} else {
		int sel = qMax(titles.indexOf(save), 0);
		ui->inputComPortList->setCurrentIndex(sel);
	}
}

void BurningControlWindow::on_buttonStartAuto_clicked(bool checked) {
	// TODO
	ui->btnOpenSettings->setEnabled(!checked);
}

void BurningControlWindow::on_btnStartBurn_clicked() {
	QString comPort = ui->inputComPortList->currentText();
	if (portList.contains(comPort)) {
		comPort = portList[comPort];
	}
	if (comPort.isEmpty()) {
		return;
	}

	auto request = new K510BurningRequest();

	request->comPort = comPort;

	emit newProcessRequested(request);
}
