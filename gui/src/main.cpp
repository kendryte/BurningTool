#include "main.h"
#include "MainWindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

QApplication *a;
QTranslator translator;
int main(int argc, char *argv[]) {
	a = new QApplication(argc, argv);

	const QStringList uiLanguages = QLocale::system().uiLanguages();
	for (const QString &locale : uiLanguages) {
		const QString baseName = "buringtool-qt_" + QLocale(locale).name();
		if (translator.load(":/i18n/" + baseName)) {
			a->installTranslator(&translator);
			break;
		}
	}

	MainWindow w;
	w.show();

	auto r = a->exec();

	qDebug("Application main() return.");

	QApplication::exit(r);

	return r;
}

QString tr(const char *s, const char *c, int n) {
	return a->tr(s, c, n);
}
