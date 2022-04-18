#include "UpdateChecker.h"
#include "BurnLibrary.h"
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
#include <QThreadPool>
#include <QTimer>

UpdateChecker::UpdateChecker(UpdateButton *button) : button(button) {
	setAutoDelete(false);
	connect(this, &UpdateChecker::giveTip, button, &UpdateButton::changeTitle);
	QTimer::singleShot(10000, [=] { QThreadPool::globalInstance()->start(this); });
}

void UpdateChecker::run() {
	try {
		_run();
	} catch (QException e) {
		emit giveTip(::tr("更新出错"));
	}
}

void UpdateChecker::_run() {
	emit giveTip(::tr("正在检查更新"));

	QNetworkAccessManager mgr;
	QNetworkRequest request{QUrl("https://api.github.com/repos/kendryte/BurningTool/releases/latest")};
	request.setHeader(QNetworkRequest::UserAgentHeader, "kendryte/BurningTool " VERSION_STRING);

	QNetworkReply *reply = mgr.get(request);

	QEventLoop eventLoop;
	QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
	eventLoop.exec();

	if (reply->error() != QNetworkReply::NoError) {
		BurnLibrary::instance()->onDebugLog(false, ::tr("无法检查更新: ") + reply->errorString());
		emit giveTip(::tr("无法检查更新: ") + QString::number(reply->error()));
		return;
	}

	QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
	QString sha = jsonResponse.object().value("target_commitish").toString();
	BurnLibrary::instance()->localLog(QStringLiteral("newest version is: ") + sha);
	BurnLibrary::instance()->localLog(QStringLiteral("my     version is: ") + QString::fromLatin1(VERSION_HASH));

	if (sha != VERSION_HASH) {
		emit giveTip(::tr("←发现更新"));
	} else {
		emit giveTip(::tr("没有更新"));
	}
}
