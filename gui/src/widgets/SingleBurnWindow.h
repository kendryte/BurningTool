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

	const bool isAutoCreate;
	Ui::SingleBurnWindow *ui;
	class BurningProcess *work;
	QList<QMetaObject::Connection> stateConnections;
	bool isDone = false;
	class BurningRequest *request;

	void setStartState();
	void deleteWork(bool preserveRequest = false);
	void autoDismiss(bool success);

  public:
	explicit SingleBurnWindow(QWidget *parent, class BurningRequest *request);
	~SingleBurnWindow();
	void setProgressInfinit();

	auto getWork() const { return work; }
	void showEvent(QShowEvent *event);

  private slots:
	void setCompleteState();
	void setErrorState(const class KBurnException &reason);
	void setCancellingState();

	void setProgressText(const QString &tip);
	void handleDeviceStateChange();
	void bytesChanged(int maximumBytes);
	void progressChanged(int writtenBytes);

	void on_btnRetry_clicked();
	void on_btnDismiss_clicked();
	void on_btnTerminate_clicked();

  signals:
	bool retryRequested(class BurningRequest *request);
};
