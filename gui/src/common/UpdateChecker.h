#pragma once

#include <QMenu>
#include <QObject>
#include <QRunnable>

class UpdateButton : public QMenu {
    Q_OBJECT

  public:
    UpdateButton(QWidget *parent = nullptr) : QMenu(parent){};

  public slots:
    void changeTitle(QString newTitle) { setTitle(newTitle); };
};

class UpdateChecker : public QObject, public QRunnable {
	Q_OBJECT

	UpdateButton *button;
	void _run();

  signals:
    void giveTip(QString tip);

  public:
    explicit UpdateChecker(UpdateButton *button);

    void run();
};
