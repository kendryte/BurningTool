#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QFile>
#include <QSettings>
#include <QWidget>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QWidget {
	Q_OBJECT
	QSettings settings;
	QFile fd;

  public:
	explicit SettingsWindow(QWidget *parent = nullptr);
	~SettingsWindow();

	const QFile &getFile() { return fd; }

  signals:
	void sysImageSelected(const QFile &fd);

  private slots:
	void loadSysImage();
	void on_btnSelectImage_clicked();

  private:
	Ui::SettingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
