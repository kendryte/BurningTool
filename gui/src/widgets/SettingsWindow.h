#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include "common/SettingsHelper.h"
#include <QFile>
#include <QSettings>
#include <QWidget>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QWidget {
	Q_OBJECT
	QFile fd;
	Ui::SettingsWindow *ui;
	QSettings settings;
	bool shown = false;

  public:
	explicit SettingsWindow(QWidget *parent = nullptr);
	~SettingsWindow();

	QString getFile() { return fd.fileName(); }
	void showEvent(QShowEvent *event);

  signals:
	void settingsCommited();
	void settingsUnsaved(bool hasUnsave);

	void updateCheckboxState();

  public slots:
	bool checkSysImage();

  private slots:
	void on_btnSelectImage_clicked();
	void acceptSave();
	void restoreDefaults();

  private:
	void reloadSettings();
	void checkAndSetInt(const QString &setting, const QString &input);
	template <typename keyT, typename valueT>
	void checkAndSetMap(const QMap<keyT, valueT> &map, const char *config, keyT current);
	void connectCheckboxEnable(SettingsBool &setting, class QCheckBox *source, class QWidget *target);
};

#endif // SETTINGSWINDOW_H
