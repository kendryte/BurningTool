#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->mainTabView->removeTab(3);
	ui->mainTabView->removeTab(2);
	ui->mainTabView->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
	delete ui;
}

#include <qfiledialog.h>
void MainWindow::on_btnSelectImage_clicked()
{
	auto str = QFileDialog::getOpenFileName(this, tr("打开系统镜像"), tr(""));
	if (str.isEmpty())
		return;
	ui->inputSysImage->setText(str);
}

void MainWindow::on_inputSysImage_returnPressed()
{

}


void MainWindow::on_btnOpenWebsite_triggered()
{

}


void MainWindow::on_actionw_triggered()
{

}

