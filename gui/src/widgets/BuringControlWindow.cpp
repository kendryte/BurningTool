#include "BuringControlWindow.h"
#include "common/BuringRequest.h"
#include "common/BurnLibrary.h"
#include "ui_BuringControlWindow.h"

BuringControlWindow::BuringControlWindow(QWidget *parent) : QGroupBox(parent), ui(new Ui::BuringControlWindow) {
	ui->setupUi(this);

	connect(BurnLibrary::instance(), &BurnLibrary::onSerialPortList, this, &BuringControlWindow::handleSerialPortList);
}

BuringControlWindow::~BuringControlWindow() {
	delete ui;
}

void BuringControlWindow::handleSerialPortList(const QMap<QString, QString> &list) {
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

void BuringControlWindow::on_buttonStartAuto_clicked(bool checked) {
	// TODO
}

void BuringControlWindow::on_btnStartBurn_clicked() {
	QString comPort = ui->inputComPortList->currentText();
	if (portList.contains(comPort)) {
		comPort = portList[comPort];
	}

	auto request = new K510BuringRequest();

	request->comPort = comPort;

	emit newProcessRequested(request);
}
