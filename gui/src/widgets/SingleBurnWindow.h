#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QFutureWatcher>
#include <QList>
#include <QWidget>

namespace Ui {
class SingleBurnWindow;
}

class SingleBurnWindow : public QWidget {
	Q_OBJECT

	Ui::SingleBurnWindow *ui;
	class BurningProcess *work;
	QList<QMetaObject::Connection> stateConnections;
	bool isDone = false;
	bool autoDismiss = false;

	void attach(BurningProcess *work);
	void setStartState();

  public:
	explicit SingleBurnWindow(QWidget *parent, class BurningProcess *work);
	~SingleBurnWindow();
	void setProgressInfinit();
	void setAutoDismiss(bool autodis);

	auto getWork() const { return work; }

	void setSize(int size);

  private slots:
	void setCompleteState();
	void setErrorState(const class KBurnException &reason);
	void setCancellingState();

	void setProgressText(const QString &tip);
	void handleDeviceStateChange(const struct kburnDeviceNode *dev);

	void on_btnRetry_clicked();
	void on_btnDismiss_clicked();
	void on_btnTerminate_clicked();

  signals:
	void retryRequested();
};
