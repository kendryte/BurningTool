#include "SingleBurnWindow.h"
#include "common/BurnLibrary.h"
#include "ui_SingleBurnWindow.h"

SingleBurnWindow::SingleBurnWindow(QWidget *parent) : QWidget(parent), ui(new Ui::SingleBurnWindow) {
	ui->setupUi(this);
}

SingleBurnWindow::~SingleBurnWindow() {
	delete ui;
}

void SingleBurnWindow::setLibrary(class BurnLibrary *library) {
	QObject::connect(library, &BurnLibrary::onSerialPortList, this, &SingleBurnWindow::handleSerialPortList);
	QObject::connect(library, &BurnLibrary::onHandleSerial, this, &SingleBurnWindow::handleDeviceStateChange);
	QObject::connect(library, &BurnLibrary::onHandleUsb, this, &SingleBurnWindow::handleDeviceStateChange);

	QObject::connect(this, &SingleBurnWindow::startBurn, library, &BurnLibrary::startBurn);
}
void SingleBurnWindow::on_btnStartBurn_clicked() {
	emit startBurn(ui->inputComPortList->currentText());
}

void SingleBurnWindow::handleSerialPortList(const QStringList &list) {
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
	ui->inputComPortList->addItems(list);
	if (custom) {
		ui->inputComPortList->setEditText(save);
	} else {
		int sel = qMax(list.indexOf(save), 0);
		ui->inputComPortList->setCurrentIndex(sel);
	}
}

void SingleBurnWindow::handleDeviceStateChange(kburnDeviceNode *dev) {
	QString val;
	val += tr("Serial Device: ");
	if (dev->serial->init || dev->serial->isUsbBound) {
		val += dev->serial->deviceInfo.path;
		val += '\n';
		val += tr("  * init: ");
		val += dev->serial->init ? tr("yes") : tr("no");
		val += '\n';
		val += tr("  * isOpen: ");
		val += dev->serial->isOpen ? tr("yes") : tr("no");
		val += '\n';
		val += tr("  * isConfirm: ");
		val += dev->serial->isConfirm ? tr("yes") : tr("no");
		val += '\n';
		val += tr("  * isUsbBound: ");
		val += dev->serial->isUsbBound ? tr("yes") : tr("no");
	} else {
		val += tr("not connected");
	}
	val += '\n';

	val += tr("Usb Device: ");
	if (dev->usb->init) {
		for (auto i = 0; i < MAX_USB_PATH_LENGTH - 1; i++) {
			val += QString::number(dev->usb->deviceInfo.path[i], 16) + QChar(':');
		}
		val.chop(1);
		val += '\n';

		val += "  * VID: " + QString::number(dev->usb->deviceInfo.idVendor, 16) + '\n';
		val += "  * PID: " + QString::number(dev->usb->deviceInfo.idProduct, 16) + '\n';
	} else {
		val += tr("not connected");
	}
	val += '\n';

	if (dev->error->code) {
		val += tr("  * error status: ");
		auto errs = kburnSplitErrorCode(dev->error->code);
		val += tr("kind: ");
		val += QString::number(errs.kind >> 32);
		val += ", ";
		val += tr("code: ");
		val += QString::number(errs.code);
		val += ", ";
		val += QString::fromLatin1(dev->error->errorMessage);
	}

	ui->textPortInfo->setText(val);
}
