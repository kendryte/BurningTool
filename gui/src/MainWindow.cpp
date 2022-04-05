#include "MainWindow.h"
#include "common/BurnLibrary.h"
#include "main.h"
#include "ui_MainWindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	ui->mainTabView->removeTab(3);
	ui->mainTabView->removeTab(2);
	ui->mainTabView->setCurrentIndex(0);

	kburn_err_t err = kburnCreate(&context);
	if (err != KBurnNoErr) {
		QMessageBox msg(QMessageBox::Icon::Critical, tr("错误"), tr("无法初始化读写功能"), QMessageBox::StandardButton::Close);
		msg.exec();
		this->close();
	}

	logReceiver = new BurnLibrary(context);
	QObject::connect(logReceiver, &BurnLibrary::onDebugLog, this->ui->textLog, &LoggerWindow::append);

	// kburnStartWaitingDevices(kb_context);
	// connect(this->ui, SIGNAL(destroy()), logReceiver, SLOT(xxx));
}

MainWindow::~MainWindow() {
	kburnGlobalDestroy();
	delete logReceiver;
	delete ui;
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

bool MainWindow::handleDeviceConnected(kburnDeviceNode *dev) { return false; }
