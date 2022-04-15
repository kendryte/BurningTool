#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>

#define SETTINGS_BASE(Class, Type)                               \
  public:                                                        \
	Q_SLOT void setValue(Type v) { SettingsBase::setValue(v); }; \
	Q_SIGNAL void changed(Type);                                 \
                                                                 \
  private:

namespace GlobalSetting {
extern QList<QString> knownCategory;
}

template <class Type>
class SettingsBase {
	const QString field;
	const Type defaultValue;
	Type _cache_val;

  protected:
	QSettings settings;

  public:
	explicit SettingsBase(const QString &category, const QString &field, const Type defaultValue)
		: settings{QSettings::Scope::UserScope, SETTINGS_CATEGORY, category}, field(field), defaultValue(defaultValue) {
		QVariant qv = settings.value(field, defaultValue);
		_cache_val = qv.value<Type>();
		if (!GlobalSetting::knownCategory.contains(category)) {
			GlobalSetting::knownCategory.append(category);
		}
	};
	Type getValue() const { return _cache_val; }

	void setValue(const Type &val) {
		_cache_val = val;
		emit changed(val);
		return settings.setValue(field, val);
	}

	void reset() { setValue(defaultValue); }

	virtual void changed(Type) = 0;
};

class SettingsBool : public QObject, public SettingsBase<bool> {
	SETTINGS_BASE(SettingsBool, bool)
	Q_OBJECT

  public:
	using SettingsBase::SettingsBase;
	void connectAction(class QAction *action);
	void connectCheckBox(class QCheckBox *input);
};

class SettingsUInt : public QObject, public SettingsBase<uint> {
	SETTINGS_BASE(SettingsUInt, uint)
	Q_OBJECT

  public:
	using SettingsBase::SettingsBase;
	void connectSpinBox(class QSpinBox *input);
};
