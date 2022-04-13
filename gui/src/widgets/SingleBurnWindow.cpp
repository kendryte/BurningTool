#include "SingleBurnWindow.h"
#include "common/BurnLibrary.h"
#include "ui_SingleBurnWindow.h"

SingleBurnWindow::SingleBurnWindow(QWidget *parent) : QWidget(parent), ui(new Ui::SingleBurnWindow) {
	ui->setupUi(this);

	QSizePolicy sp_retain = ui->textStatus->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->textStatus->setSizePolicy(sp_retain);

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
	QObject::connect(this, &SingleBurnWindow::startedBurn, library, &BurnLibrary::startBurn);
	QObject::connect(ui->inputComPortList, &QComboBox::currentTextChanged, this, &SingleBurnWindow::resetProgressState);
}

void SingleBurnWindow::on_btnStartBurn_clicked() {
	if (work) {
		// ????
		qErrnoWarning("Invalid state: re-run on_btnStartBurn_clicked; this is impossible\n");
	}

	work = emit startedBurn(ui->inputComPortList->currentText());
	if (work == NULL) {
		ui->textStatus->failed(tr("发生未知错误"));
		return;
	}

	connect(work, &FlashTask::onDeviceChange, this, &SingleBurnWindow::handleDeviceStateChange);
	connect(work, &FlashTask::progressTextChanged, this, &SingleBurnWindow::setProgressText);

	resetProgressState();
	ui->textStatus->message(tr("烧录中..."));

	setEnabled(false);

	future = new QFutureWatcher<void>();

	connect(future, &QFutureWatcher<void>::finished, this, &SingleBurnWindow::setCompleteState);
	connect(future, &QFutureWatcher<void>::canceled, this, &SingleBurnWindow::setErrorState);
	connect(future, &QFutureWatcher<void>::progressRangeChanged, ui->progressBar, &QProgressBar::setRange);
	connect(future, &QFutureWatcher<void>::progressValueChanged, ui->progressBar, &QProgressBar::setValue);

	future->setFuture(work->future());
}

void SingleBurnWindow::setProgressText(const QString &progressText) {
#ifdef WIN32
	ui->textStatus->message(progressText);
#else
	if (progressText.isEmpty()) {
		ui->progressBar->setFormat("%p%");
	} else {
		ui->progressBar->setFormat(progressText + ": %p%");
	}
#endif
}

void SingleBurnWindow::setConfigureStata(bool incomplete) {
	if (work) {
		return;
	}
	if (incomplete) {
		ui->textStatus->message(tr("需要保存设置"));
	} else {
		ui->textStatus->standby(tr("就绪"));
	}
	setDisabled(incomplete);
}

void SingleBurnWindow::setCompleteState() {
	ui->textStatus->success(tr("完成！"));
	resumeState();
	emit completedBurn(true);
}

void SingleBurnWindow::setErrorState() {
	auto r = work->getResult();
	auto e = kburnSplitErrorCode(r->errorCode);
	ui->textStatus->failed(tr("错误: ") + QString::number(e.kind >> 32) + " - (" + QString::number(e.code, 16) + "), " + r->errorMessage);

	resumeState();
	emit completedBurn(false);
}

void SingleBurnWindow::resetProgressState() {
	setProgressText("");
	ui->textStatus->standby(tr("就绪"));
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
