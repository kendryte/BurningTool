#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QFile>
#include <QSettings>
#include <QWidget>

namespace Ui {
class SettingsWindow;
}

#define SETTINGS_CATEGORY "kendryte"

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
	void settingsChanged();
	void settingsUnsaved(bool hasUnsave);

  private slots:
	bool checkSysImage();
	void on_btnSelectImage_clicked();
	void on_inputSysImage_editingFinished();
	void on_actionBar_clicked(class QAbstractButton *button);
	void acceptSave();
	void reloadSettings();
	void inputChanged();

  private:
	void restoreDefaults();

	void checkAndSetInt(const QString &setting, const QString &input);
	template <typename keyT, typename valueT>
	void checkAndSetMap(const QMap<keyT, valueT> &map, const char *config, keyT current);
};

#endif // SETTINGSWINDOW_H
