#include "SingleBurnWindow.h"
#include "common/AppGlobalSetting.h"
#include "common/BurningProcess.h"
#include "common/BurningRequest.h"
#include "common/BurnLibrary.h"
#include "common/MyException.h"
#include "ui_SingleBurnWindow.h"

SingleBurnWindow::SingleBurnWindow(QWidget *parent, BurningRequest *request)
	: QWidget(parent), ui(new Ui::SingleBurnWindow), request(request), isAutoCreate(request->isAutoCreate) {
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
		auto instance = BurnLibrary::instance();
		if (instance) { // 为了退出时主窗口不消失，library析构更早
			instance->deleteBurning(work);
		}
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

	autoDismiss(true);
}

void SingleBurnWindow::setErrorState(const KBurnException &reason) {
	auto e = kburnSplitErrorCode(reason.errorCode);
	ui->textStatus->failed(tr("错误: ") + QString::number(e.kind >> 32) + " - (" + QString::number(e.code, 16) + "), " + reason.errorMessage);

	ui->progressBar->hide();
	ui->textStatus->show();
	ui->btnDismiss->show();
	ui->btnRetry->show();
	ui->btnTerminate->hide();

	autoDismiss(false);
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

void SingleBurnWindow::handleDeviceStateChange() {
	QString val;
	setToolTip(work->getDetailInfo());
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

void SingleBurnWindow::autoDismiss(bool success) {
	if (!GlobalSetting::autoConfirm.getValue()) {
		return;
	}
	if (!isAutoCreate && !GlobalSetting::autoConfirmManualJob.getValue()) {
		return;
	}
	if (!success && !GlobalSetting::autoConfirmEvenError.getValue()) {
		return;
	}

	ui->btnDismiss->show();
	ui->btnDismiss->setDisabled(true);
	ui->btnRetry->hide();

	auto tmr = new QTimer(this);
	tmr->setInterval(1000);
	int *i = new int(5);
	auto cb = [=] {
		ui->btnDismiss->setText(tr("确认") + " (" + QString::number(*i) + ")");
		if (*i == 0) {
			tmr->stop();
			deleteLater();
		}
		(*i)--;
	};
	connect(tmr, &QTimer::timeout, this, cb);
	cb();
	tmr->start();

	connect(tmr, &QTimer::destroyed, [=] { delete i; });
	connect(this, &SingleBurnWindow::destroyed, tmr, &QTimer::deleteLater);
}
