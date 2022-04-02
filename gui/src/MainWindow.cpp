#include "MainWindow.h"
#include "common/logger.h"
#include "main.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->mainTabView->removeTab(3);
    ui->mainTabView->removeTab(2);
    ui->mainTabView->setCurrentIndex(0);

    logReceiver = new LogReceiver();
    QObject::connect(logReceiver, &LogReceiver::onReceiveLine, this->ui->textLog, &LoggerWindow::append);

    kburnOnSerialConnect(kb_context, reinterpret_cast<on_device_connect>(&MainWindow::handleDeviceConnected), this);

    kburnStartWaitingDevices(kb_context);
    // connect(this->ui, SIGNAL(destroy()), logReceiver, SLOT(xxx));
}

MainWindow::~MainWindow() {
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
