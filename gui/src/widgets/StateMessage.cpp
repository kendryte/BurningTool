#include "StateMessage.h"
#include <QGraphicsOpacityEffect>

static const QString successStyle = "color: #04ba13; font-weight: bold;";
static const QString failedStyle = "color: #ff1919; font-weight: bold;";
static const QString standbyStyle = "font-weight: bold;";
static const QString normalStyle = "font-size: 20px;";

StateMessage::StateMessage(QWidget *parent) : QScrollArea(parent) {
	// TODO: 较长消息想办法显示全
	setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	opacity = new QGraphicsOpacityEffect(this);
	opacity->setOpacity(1.00);
	label.setGraphicsEffect(opacity);
	label.setAutoFillBackground(true);

	setWidgetResizable(true);
	setWidget(&label);

	setFrameShape(QFrame::NoFrame);

	label.show();

	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void StateMessage::setState(const QString &status, const QString &message) {
	label.setStyleSheet(QStringLiteral("QLabel{") + status + "}");
	setToolTip(message);
	resize();
}

QSize StateMessage::sizeHint() const {
	return QSize(0, label.sizeHint().height());
}

void StateMessage::resize() {
	auto msg = toolTip();
	QFontMetrics metrics(font());
	QString elidedText = metrics.elidedText(msg, Qt::ElideRight, width() - 10);
	label.setText(elidedText);
}

void StateMessage::timerEvent(QTimerEvent *event) {
	hidden = !hidden;
	if (hidden) {
		opacity->setOpacity(0);
	} else {
		opacity->setOpacity(1);
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
	// FIXME: thread issue
	if (b) {
		if (!timer) {
			// timer = startTimer(1000);
		}
	} else {
		if (timer) {
			// killTimer(timer);
			timer = 0;
			show();
		}
	}
}
