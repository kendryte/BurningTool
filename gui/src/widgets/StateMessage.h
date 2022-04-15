#pragma once

#include "qtextedit.h"
#include <QLabel>
#include <QScrollArea>
#include <QStyle>
#include <QTimer>

class StateMessage : public QScrollArea {
  private:
    class QGraphicsOpacityEffect *opacity;
    QLabel label;

	int timer = 0;
	bool hidden = false;

	void timerEvent(QTimerEvent *event);
	void blink(bool start);
	void setState(const QString &status, const QString &message);
	void resize();

  public:
    void resizeEvent(QResizeEvent *) { resize(); }
    QSize sizeHint() const override;

	StateMessage(QWidget *parent);
	void success(const QString &message);
	void failed(const QString &message);
	void standby(const QString &message);
	void message(const QString &message);
};
