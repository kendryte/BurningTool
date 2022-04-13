#pragma once

#include "qtextedit.h"
#include <QLabel>
#include <QStyle>
#include <QTimer>

class StateMessage : public QLabel {
  private:
	using QLabel::setStyleSheet;
	using QLabel::setText;

	int timer = 0;
	void timerEvent(QTimerEvent *event);
	void blink(bool start);
	void setState(const QString &status, const QString &message);

  public:
	StateMessage(QWidget *parent);
	void success(const QString &message);
	void failed(const QString &message);
	void standby(const QString &message);
	void message(const QString &message);
};
