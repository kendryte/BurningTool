#include "StateMessage.h"

static const QString successStyle = "color: #04ba13; font-weight: bold;";
static const QString failedStyle = "color: #ff1919; font-weight: bold;";
static const QString standbyStyle = "font-weight: bold;";
static const QString normalStyle = "font-size: 20px;";

StateMessage::StateMessage(QWidget *parent) : QLabel(parent) {
	setAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void StateMessage::setState(const QString &status, const QString &message) {
	setStyleSheet(QStringLiteral("QLabel{") + status + "}");
	setText(message);
}

void StateMessage::timerEvent(QTimerEvent *event) {
	if (isHidden()) {
		show();
	} else {
		hide();
	}
}

void StateMessage::success(const QString &message) {
	setState(normalStyle + successStyle, message);
	blink(true);
}

void StateMessage::failed(const QString &message) {
	setState(normalStyle + failedStyle, message);
	blink(true);
}

void StateMessage::standby(const QString &message) {
	setState(normalStyle + standbyStyle, message);
	blink(true);
}

void StateMessage::message(const QString &message) {
	setState(normalStyle, message);
	blink(false);
	show();
}

void StateMessage::blink(bool b) {
	if (b) {
		if (!timer) {
			timer = startTimer(1000);
		}
	} else {
		if (timer) {
			killTimer(timer);
			timer = 0;
			show();
		}
	}
}
