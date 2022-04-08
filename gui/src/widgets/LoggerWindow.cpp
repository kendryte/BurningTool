#include "LoggerWindow.h"
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QScrollBar>

LoggerWindow::LoggerWindow(QWidget *parent) : QTextEdit(parent) { autoScroll = false; }

LoggerWindow::~LoggerWindow() {}

void LoggerWindow::scrollToBottom() {
	if (autoScroll) {
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	}
}

void LoggerWindow::append(const QString &line) {
	QTextEdit::append("<div style=\"white-space:pre;\">" + line + "</div>");
	scrollToBottom();
}

void LoggerWindow::resizeEvent(QResizeEvent *e) {
	QTextEdit::resizeEvent(e);
	scrollToBottom();
}

void LoggerWindow::contextMenuEvent(QContextMenuEvent *event) {
	QMenu *menu = createStandardContextMenu();
	menu->addSeparator();

	QAction *act = menu->addAction(tr("Auto Scroll"), this, &LoggerWindow::toggleAutoScroll);
	act->setCheckable(true);
	act->setChecked(autoScroll);

	QAction *act2 = menu->addAction(tr("Clear"), this, &LoggerWindow::clearScreen);

	menu->exec(event->globalPos());
	delete menu;
}
