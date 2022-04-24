#include "LoggerWindow.h"
#include "config.h"
#include <QAction>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QMenu>
#include <QScrollBar>
#include <QString>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x

LoggerWindow::LoggerWindow(QWidget *parent) : QTextEdit(parent) {
	logfile.setFileName(QCoreApplication::applicationDirPath() + "/burning_tool.html");
	logfile.open(QIODeviceBase::Unbuffered | QIODeviceBase::Truncate | QIODeviceBase::WriteOnly);

	logfile.write(QStringLiteral("<div>IS_CI=" STRINGIZE(IS_CI) "</div>").toUtf8());
	logfile.write(QStringLiteral("<div>VERSION_STRING=" VERSION_STRING "</div>").toUtf8());
	logfile.write(QStringLiteral("<div>VERSION_HASH=" VERSION_HASH "</div>").toUtf8());
	logfile.write("\n");
}

LoggerWindow::~LoggerWindow() {
}

bool LoggerWindow::copyLogFileTo(const QString &target) {
	// 这个函数必须在ui线程中调用
	logfile.close();

	QFile dist(target);
	if(dist.exists()){
		dist.remove();
	}
	return logfile.copy(target);
}

void LoggerWindow::scrollToBottom() {
	if (autoScroll) {
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
		horizontalScrollBar()->setValue(0);
	}
}

void LoggerWindow::append(bool isTrace, const QString &line) {
	QString newLine = QString::fromLatin1("<div style=\"white-space:pre;\">") + line + "</div>";
	logfile.write((newLine + "\n").toUtf8());

	if (isTrace && !trace_visible) {
		return;
	}

	QTextEdit::append(newLine);

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
