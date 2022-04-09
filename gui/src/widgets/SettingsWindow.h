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
	QList<uint32_t> baudrates = {9600, 115200, 230400, 460800, 576000, 921600};
	QSettings settings;

  public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    QString getFile() { return fd.fileName(); }

  signals:
    void settingsChanged();

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
