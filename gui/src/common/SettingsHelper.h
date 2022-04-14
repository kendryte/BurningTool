#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>

class SettingsBool : public QObject {
	Q_OBJECT

	const QString field;
	const QVariant defaultValue;
	QSettings settings;

  public:
	SettingsBool(const QString &category, const QString &field, const bool defaultValue);
	bool getValue() const;
	void connectAction(class QAction *action);

  public slots:
	void setValue(bool);

  signals:
	void changed(bool);
};
