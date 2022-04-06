#include "MainWindow.h"
#include "common/BurnLibrary.h"
#include "main.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	ui->mainTabView->removeTab(3);
	ui->mainTabView->removeTab(2);
	ui->mainTabView->setCurrentIndex(0);

	kburn_err_t err = kburnCreate(&context);
	if (err != KBurnNoErr)
		fatalAlert(err);

	library = new BurnLibrary(context);
	QObject::connect(library, &BurnLibrary::onDebugLog, this->ui->textLog, &LoggerWindow::append);
	QObject::connect(library, &BurnLibrary::onSerialPortList, this, &MainWindow::handleSerialPortList);
	QObject::connect(library, &BurnLibrary::onHandleSerial, this, &MainWindow::handleDeviceStateChange);
	QObject::connect(library, &BurnLibrary::onHandleUsb, this, &MainWindow::handleDeviceStateChange);

	// kburnStartWaitingDevices(kb_context);
	// connect(this->ui, SIGNAL(destroy()), logReceiver, SLOT(xxx));
}

void MainWindow::showEvent(QShowEvent *event) {
	QMainWindow::showEvent(event);
	library->start();
}

MainWindow::~MainWindow() {
	delete library;
	kburnGlobalDestroy();
	delete ui;
}

void MainWindow::handleSerialPortList(const QStringList &list) {
	QString save(ui->inputComPortList->currentText());

	bool custom = true;
	for (int i = 0; i < ui->inputComPortList->count(); i++) {
		if (ui->inputComPortList->itemText(i) == save) {
			custom = false;
			break;
		}
	}

	ui->inputComPortList->clear();
	ui->inputComPortList->addItems(list);
	if (custom) {
		ui->inputComPortList->setEditText(save);
	} else {
		ui->inputComPortList->setCurrentIndex(qMin(list.indexOf(save), 0));
	}
}

#include <qfiledialog.h>
void MainWindow::on_btnSelectImage_clicked() {
	auto str = QFileDialog::getOpenFileName(this, tr("打开系统镜像"), tr(""));
	if (str.isEmpty())
		return;
	ui->inputSysImage->setText(str);
}

void MainWindow::on_inputSysImage_returnPressed() {}

void MainWindow::on_btnOpenWebsite_triggered() {}

void MainWindow::handleDeviceStateChange(kburnDeviceNode *dev) {
	QString val;
	val += tr("Serial Device: ");
	if (dev->serial->init) {
		val += dev->serial->deviceInfo.path;
		val += '\n';
		val += tr("  * isOpen: ");
		val += dev->serial->isOpen ? tr("yes") : tr("no");
		val += '\n';
		val += tr("  * isConfirm: ");
		val += dev->serial->isConfirm ? tr("yes") : tr("no");
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

		val += "  * VID: " + QString::number(dev->usb->deviceInfo.idVendor, 16) + '\n';
		val += "  * VID: " + QString::number(dev->usb->deviceInfo.idProduct, 16) + '\n';
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

void MainWindow::on_btnStartBurn_clicked() { library->startBurn(ui->inputComPortList->currentText()); }
