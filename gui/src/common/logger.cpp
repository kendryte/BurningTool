#include "logger.h"
#include <QtDebug>

void LogReceiver::receive(kburnLogType level, const char *cstr)
{
	QString line(cstr);
	emit onReceiveLine(line);
}

LogReceiver::~LogReceiver()
{
	kburnSetColors(savedColors);
	kburnSetLogCallback(savedCallback.callback, savedCallback.call_context);
}

LogReceiver::LogReceiver() : console(stdout)
{
	savedColors = kburnSetColors((kburnDebugColors){
		.red = {.prefix = "<span style=\"color: red\">", .postfix = "</span>"},
		.green = {.prefix = "<span style=\"color: lime\">", .postfix = "</span>"},
		.yellow = {.prefix = "<span style=\"color: yellow\">", .postfix = "</span>"},
		.grey = {.prefix = "<span style=\"opacity: 0.5\">", .postfix = "</span>"},
	});
	savedCallback = kburnSetLogCallback(reinterpret_cast<on_debug_log>(&LogReceiver::receive), this);
}
