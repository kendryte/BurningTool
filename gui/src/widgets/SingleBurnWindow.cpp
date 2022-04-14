#include "SingleBurnWindow.h"
#include "common/BurningProcess.h"
#include "common/BurnLibrary.h"
#include "common/MyException.h"
#include "ui_SingleBurnWindow.h"

SingleBurnWindow::SingleBurnWindow(QWidget *parent, BurningProcess *work) : QWidget(parent), ui(new Ui::SingleBurnWindow), work(work) {
	ui->setupUi(this);

	setStartState();

	QSizePolicy sp_retain = ui->textStatus->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->textStatus->setSizePolicy(sp_retain);

	stateConnections << connect(work, &BurningProcess::deviceStateNotify, this, &SingleBurnWindow::handleDeviceStateChange);
	stateConnections << connect(work, &BurningProcess::stageChanged, this, &SingleBurnWindow::setProgressText);
	stateConnections << connect(work, &BurningProcess::bytesChanged, ui->progressBar, &QProgressBar::setMaximum);
	stateConnections << connect(work, &BurningProcess::progressChanged, ui->progressBar, &QProgressBar::setValue);

	connect(work, &BurningProcess::completed, this, &SingleBurnWindow::setCompleteState);
	connect(work, &BurningProcess::failed, this, &SingleBurnWindow::setErrorState);
	connect(work, &BurningProcess::cancelRequested, this, &SingleBurnWindow::setCancellingState);
	connect(work, &BurningProcess::destroyed, this, &QWidget::deleteLater);
}

SingleBurnWindow::~SingleBurnWindow() {
	BurnLibrary::instance()->deleteBurning(work);
	work = NULL;
	delete ui;
}

void SingleBurnWindow::setSize(int size) {
	setFixedWidth(size);
}

void SingleBurnWindow::setStartState() {
	setProgressText("");
	setProgressInfinit();

	ui->progressBar->show();
	ui->textStatus->hide();
	ui->btnDismiss->hide();
	ui->btnRetry->hide();
	ui->btnTerminate->show();
}

void SingleBurnWindow::setCompleteState() {
	ui->textStatus->success(tr("完成！"));

	ui->progressBar->hide();
	ui->textStatus->show();
	ui->btnDismiss->show();
	ui->btnRetry->hide();
	ui->btnTerminate->hide();

	if (autoDismiss) {
		deleteLater();
	}
}

void SingleBurnWindow::setErrorState(const KBurnException &reason) {
	auto e = kburnSplitErrorCode(reason.errorCode);
	ui->textStatus->failed(tr("错误: ") + QString::number(e.kind >> 32) + " - (" + QString::number(e.code, 16) + "), " + reason.errorMessage);

	ui->progressBar->hide();
	ui->textStatus->show();
	ui->btnDismiss->show();
	ui->btnRetry->show();
	ui->btnTerminate->hide();
}

void SingleBurnWindow::setCancellingState() {
	for (auto conn : stateConnections) {
		QObject::disconnect(conn);
	}
	ui->btnTerminate->setDisabled(true);
	setProgressText(tr("正在取消"));
	setProgressInfinit();
}

void SingleBurnWindow::setProgressText(const QString &progressText) {
	if (progressText.isEmpty()) {
		ui->progressBar->setFormat("%p%");
	} else {
		ui->progressBar->setFormat(progressText + ": %p%");
	}
}

void SingleBurnWindow::setProgressInfinit() {
	ui->progressBar->setRange(0, 0);
	ui->progressBar->setValue(0);
}

void SingleBurnWindow::setAutoDismiss(bool autodis) {
	autoDismiss = autodis;
	if (isDone) {
		deleteLater();
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

	setToolTip(val);
}

void SingleBurnWindow::on_btnRetry_clicked() {
	emit retryRequested();
	deleteLater();
}

void SingleBurnWindow::on_btnDismiss_clicked() {
	deleteLater();
}

void SingleBurnWindow::on_btnTerminate_clicked() {
	ui->btnTerminate->setDisabled(true);
	work->cancel();
}
