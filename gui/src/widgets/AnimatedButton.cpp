#include "AnimatedButton.h"
#include <QGraphicsColorizeEffect>
#include <QPainter>
#include <QPropertyAnimation>

AnimatedButton::AnimatedButton(QWidget *parent) : QPushButton(parent) {
	QColor backgroundColor = palette().light().color();

	QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
	setGraphicsEffect(effect);

	animate = new QPropertyAnimation(effect, "color");
	animate->setDuration(2000);
	effect->setColor(QColor(240, 50, 50, 0));
	animate->setKeyValueAt(0, QColor(240, 50, 50, 50));
	animate->setKeyValueAt(0.5, QColor(240, 50, 50, 255));
	animate->setKeyValueAt(1, QColor(240, 50, 50, 50));
	animate->setEasingCurve(QEasingCurve::InOutCubic);
	animate->setLoopCount(-1);

	connect(this, &AnimatedButton::clicked, [=](bool enabled) {
		if (enabled) {
			animate->start();
		} else {
			animate->stop();
			effect->setColor(QColor(240, 50, 50, 0));
		}
	});
}

AnimatedButton::~AnimatedButton() {
	delete animate;
}
