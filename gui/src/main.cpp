#include "MainWindow.h"

#include <canaan-burn/canaan-burn.h>

#include <QMessageBox>
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTranslator translator;
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for (const QString &locale : uiLanguages)
	{
		const QString baseName = "buringtool-qt_" + QLocale(locale).name();
		if (translator.load(":/i18n/" + baseName))
		{
			a.installTranslator(&translator);
			break;
		}
	}

	MainWindow w;
	w.show();

	KBCTX kb_context;
	kburn_err_t err = kburnCreate(&kb_context);
	if (err != KBurnNoErr)
	{
		QMessageBox msg(QMessageBox::Icon::Critical, translator.tr("错误"), translator.tr("无法初始化读写功能"), QMessageBox::StandardButton::Close);
		msg.exec();
		return 1;
	}

	kburnSetColors((kburnDebugColors){
		.red = {.prefix = "<span style=\"color: red\">", .postfix = "</span>"},
		.green = {.prefix = "<span style=\"color: lime\">", .postfix = "</span>"},
		.yellow = {.prefix = "<span style=\"color: yellow\">", .postfix = "</span>"},
		.grey = {.prefix = "<span style=\"opacity: 0.5\">", .postfix = "</span>"},
	});
	kburnSetLogCallback(receiver);

	int r = a.exec();

	kburnGlobalDestroy();

	return r;
}

SIGNAL log_receiver()
{
}
