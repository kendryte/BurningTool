#pragma once

#include <QObject>
#include <QString>
#include <QTextStream>
#include <canaan-burn/canaan-burn.h>

class LogReceiver : public QObject
{
	Q_OBJECT

	QTextStream console;

	struct debug_callback savedCallback;
	kburnDebugColors savedColors;

public:
	LogReceiver();
	~LogReceiver();
	void receive(kburnLogType level, const char *cstr);

signals:
	void onReceiveLine(QString line);
};
