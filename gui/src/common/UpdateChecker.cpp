#include "UpdateChecker.h"
#include "config.h"
#include "main.h"
#include <QEventLoop>
#include <QException>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRunnable>
#include <QThreadPool>

class CheckUpdate : public QRunnable {
	UpdateChecker *updateChecker;
	QMenu *button;

	void _run() {
		button->setTitle(::tr("正在检查更新"));

		QNetworkAccessManager mgr{updateChecker};
		QNetworkRequest request{QUrl("https://api.github.com/repos/kendryte/BurningTool/releases/latest")};
		request.setHeader(QNetworkRequest::UserAgentHeader, "kendryte/BurningTool " VERSION_STRING);

		QNetworkReply *reply = mgr.get(request);

		QEventLoop eventLoop;
		QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
		eventLoop.exec();

		if (reply->error() != QNetworkReply::NoError) {
			qWarning() << reply->errorString();
			button->setTitle(::tr("无法检查更新: ") + QString::number(reply->error()));
			return;
		}

		QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
		QString sha = jsonResponse.object().value("target_commitish").toString();
		qDebug() << "newest version is: " << sha;

		if (sha != VERSION_HASH) {
			qDebug() << "my version is: " VERSION_HASH;

			button->setTitle(::tr("←发现更新"));
		} else {
			button->setTitle(::tr("没有更新"));
		}
	}

  public:
    CheckUpdate(UpdateChecker *updateChecker) : updateChecker(updateChecker) { button = reinterpret_cast<QMenu *>(updateChecker->parent()); };

	void run() {
		try {
			_run();
		} catch (QException e) {
			button->setTitle(::tr("更新出错F"));
		}
	}
};

UpdateChecker::UpdateChecker(QMenu *parent) : QObject{parent} {
	QThreadPool::globalInstance()->start(new CheckUpdate(this));
}
