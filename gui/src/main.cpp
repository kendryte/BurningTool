#include "main.h"
#include "MainWindow.h"
#include <QApplication>
#include <QLocale>
#include <QMessageBox>
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

void fatalAlert(kburn_err_t err) {
	if (err != KBurnNoErr) {
		auto e = kburnSplitErrorCode(err);
		qWarning() << "error kind=" << e.kind << ", code=" << e.code << QChar('\n');
		QMessageBox msg(QMessageBox::Icon::Critical, translator.tr("错误"), translator.tr("无法初始化读写功能"), QMessageBox::StandardButton::Close);
		msg.exec();
		abort();
	}
}
