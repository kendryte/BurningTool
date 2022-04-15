#include "SingleBurnWindow.h"
#include "common/BurningProcess.h"
#include "common/BurningRequest.h"
#include "common/BurnLibrary.h"
#include "common/MyException.h"
#include "ui_SingleBurnWindow.h"

SingleBurnWindow::SingleBurnWindow(QWidget *parent, BurningRequest *request) : QWidget(parent), ui(new Ui::SingleBurnWindow), request(request) {
	ui->setupUi(this);
	hide();

	work = BurnLibrary::instance()->prepareBurning(request);
	if (!work) {
		deleteLater();
		return;
	}

	ui->textTitle->setText(work->getTitle());

	setStartState();

	QSizePolicy sp_retain = ui->textStatus->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	ui->textStatus->setSizePolicy(sp_retain);

	stateConnections << connect(work, &BurningProcess::deviceStateNotify, this, &SingleBurnWindow::handleDeviceStateChange);
	stateConnections << connect(work, &BurningProcess::stageChanged, this, &SingleBurnWindow::setProgressText);
	stateConnections << connect(work, &BurningProcess::bytesChanged, this, &SingleBurnWindow::bytesChanged);
	stateConnections << connect(work, &BurningProcess::progressChanged, this, &SingleBurnWindow::progressChanged);

	connect(work, &BurningProcess::completed, this, &SingleBurnWindow::setCompleteState);
	connect(work, &BurningProcess::failed, this, &SingleBurnWindow::setErrorState);
	connect(work, &BurningProcess::cancelRequested, this, &SingleBurnWindow::setCancellingState);
	connect(work, &BurningProcess::destroyed, this, &QWidget::deleteLater);
}

void SingleBurnWindow::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);
	if (work) {
		BurnLibrary::instance()->executeBurning(work);
	}
}

SingleBurnWindow::~SingleBurnWindow() {
	deleteWork();
	delete ui;
}

void SingleBurnWindow::bytesChanged(int maximumBytes) {
	ui->progressBar->setMaximum(maximumBytes);
}

void SingleBurnWindow::progressChanged(int writtenBytes) {
	ui->progressBar->setValue(writtenBytes);
}

void SingleBurnWindow::deleteWork(bool preserveRequest) {
	if (work) {
		work->disconnect();
		BurnLibrary::instance()->deleteBurning(work);
		work = nullptr;
	}
	if (request && !preserveRequest) {
		delete request;
	}
	request = nullptr;
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
	for (auto &conn : stateConnections) {
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
	deleteLater();
	bool someoneHandled = emit retryRequested(request);
	if (someoneHandled) {
		deleteWork(true);
	}
}

void SingleBurnWindow::on_btnDismiss_clicked() {
	deleteLater();
}

void SingleBurnWindow::on_btnTerminate_clicked() {
	ui->btnTerminate->setDisabled(true);
	work->cancel();
}
