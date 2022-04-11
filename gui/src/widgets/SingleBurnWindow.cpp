#include "SingleBurnWindow.h"
#include "common/BurnLibrary.h"
#include "ui_SingleBurnWindow.h"

SingleBurnWindow::SingleBurnWindow(QWidget *parent) : QWidget(parent), ui(new Ui::SingleBurnWindow) {
	ui->setupUi(this);

	resetProgressState();
}

SingleBurnWindow::~SingleBurnWindow() {
	if (work) {
		work->cancel();
		future->waitForFinished();
	}
	delete ui;
}

void SingleBurnWindow::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);

	if (shown)
		return;
	shown = true;

	auto library = BurnLibrary::instance();
	QObject::connect(library, &BurnLibrary::onSerialPortList, this, &SingleBurnWindow::handleSerialPortList);
	QObject::connect(this, &SingleBurnWindow::startBurn, library, &BurnLibrary::startBurn);
}

void SingleBurnWindow::on_btnStartBurn_clicked() {
	if (work) {
		// ????
		qErrnoWarning("Invalid state: re-run on_btnStartBurn_clicked; this is impossible\n");
	}
	work = emit startBurn(ui->inputComPortList->currentText());
	if (work == NULL) {
		ui->textPortInfo->setText(tr("发生未知错误"));
		return;
	}

	connect(work, &FlashTask::onDeviceChange, this, &SingleBurnWindow::handleDeviceStateChange);
	connect(work, &FlashTask::progressTextChanged, this, &SingleBurnWindow::setProgressText);

	resetProgressState();

	setEnabled(false);

	future = new QFutureWatcher<void>();

	connect(future, &QFutureWatcher<void>::finished, this, &SingleBurnWindow::resumeState);
	connect(future, &QFutureWatcher<void>::finished, this, &SingleBurnWindow::resetProgressState);
	connect(future, &QFutureWatcher<void>::canceled, this, [=]() {
		auto r = work->getResult();
		auto e = kburnSplitErrorCode(r->errorCode);
		ui->textStatus->setText(tr("错误: ") + QString::number(e.kind >> 32) + " - (" + QString::number(e.code, 16) + "), " + r->errorMessage);
		ui->textStatus->setStyleSheet("QLabel { color : red; }");

		resumeState();
	});
	connect(future, &QFutureWatcher<void>::progressRangeChanged, ui->progressBar, &QProgressBar::setRange);
	connect(future, &QFutureWatcher<void>::progressValueChanged, ui->progressBar, &QProgressBar::setValue);

	future->setFuture(work->future());
}

void SingleBurnWindow::setProgressText(const QString &progressText) {
#ifdef WIN32
	ui->textStatus->setText(progressText);
#else
	if (progressText.isEmpty()) {
		ui->progressBar->setFormat("%p%");
	} else {
		ui->progressBar->setFormat(progressText + ": %p%");
	}
#endif
}

void SingleBurnWindow::resetProgressState() {
	setProgressText("");
	ui->textStatus->setStyleSheet("QLabel {}");

#ifdef WIN32
	ui->textStatus->setText(tr("就绪"));
#endif

	ui->textPortInfo->setText("");
	ui->progressBar->setRange(0, 100);
	ui->progressBar->setValue(0);
}

void SingleBurnWindow::resumeState() {
	delete future;
	future = NULL;
	delete work;
	work = NULL;
	setEnabled(true);
}

void SingleBurnWindow::setEnabled(bool enabled) {
	ui->btnStartBurn->setEnabled(enabled);
	ui->inputComPortList->setEnabled(enabled);
}

void SingleBurnWindow::handleSerialPortList(const QMap<QString, QString> &list) {
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

void SingleBurnWindow::handleDeviceStateChange(const kburnDeviceNode *dev) {
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
