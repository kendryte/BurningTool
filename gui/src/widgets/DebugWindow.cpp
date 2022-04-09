#include "DebugWindow.h"
#include "ui_DebugWindow.h"

DebugWindow::DebugWindow(QWidget *parent) : QWidget(parent), ui(new Ui::DebugWindow) {
	ui->setupUi(this);
}

DebugWindow::~DebugWindow() {
	delete ui;
}
