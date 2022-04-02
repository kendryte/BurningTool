#include "main.h"
#include "MainWindow.h"
#include <QApplication>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

KBCTX kb_context;

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);

	QTranslator translator;
	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for (const QString &locale : uiLanguages) {
		const QString baseName = "buringtool-qt_" + QLocale(locale).name();
		if (translator.load(":/i18n/" + baseName)) {
			a.installTranslator(&translator);
			break;
		}
	}

	kburn_err_t err = kburnCreate(&kb_context);
	if (err != KBurnNoErr) {
		QMessageBox msg(QMessageBox::Icon::Critical, translator.tr("错误"), translator.tr("无法初始化读写功能"), QMessageBox::StandardButton::Close);
		msg.exec();
		return 1;
	}

	MainWindow w;
	w.show();

	int r = a.exec();

	kburnGlobalDestroy();

	return r;
}
