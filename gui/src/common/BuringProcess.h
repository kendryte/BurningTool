#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QRunnable>
#include <QString>

void test(void);

class FlashTask : public QRunnable {
  private:
	QString systemImage;
	FlashTask();
	KBCTX scope;

  public:
	FlashTask(KBCTX scope) : scope(scope){};
	void setSystemImageFile(const QString &systemImage);
	void run();

  signals:

  public slots:
};
