#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>

#define SETTINGS_BASE(Class, Type)                                     \
  public:                                                              \
	Q_SLOT void setValue(const Type &v) { SettingsBase::setValue(v); } \
	Q_SIGNAL void changed(Type);                                       \
                                                                       \
	template <typename Func2>                                          \
	void connectLambda(QObject *bind, Func2 lambda) {                  \
		QMetaObject::invokeMethod(bind, [=] { lambda(getValue()); });  \
		connect(this, &Class::changed, bind, lambda);                  \
	}                                                                  \
                                                                       \
  private:                                                             \
	Q_SLOT void setValueImmediate(const Type &v) { SettingsBase<Type>::setValueImmediate(v); }

class ISettingsBase {
  protected:
	explicit ISettingsBase(){};
	virtual void clear() = 0;
	virtual bool commit() = 0;

  public:
	static QList<ISettingsBase *> settingsRegistry;
	static void resetEverythingToDefaults();
	static void commitAllSettings();
};

template <class Type>
class SettingsBase : public ISettingsBase {
	const QString field;
	Type cachedValue;
	Type activeValue;

  protected:
	const Type defaultValue;
	QSettings settings;
	Type getCacheValue() const { return cachedValue; }
	void setValueImmediate(const Type &val) {
		setValue(val);
		commit();
	}

  public:
	explicit SettingsBase(const QString &category, const QString &field, const Type defaultValue)
		: settings{QSettings::Scope::UserScope, SETTINGS_CATEGORY, category}, field(field), defaultValue(defaultValue) {
		settingsRegistry.append(this);

		QVariant qv = settings.value(field, defaultValue);
		cachedValue = qv.value<Type>();
		activeValue = cachedValue;
	}
	~SettingsBase() { settingsRegistry.removeOne(this); }

	Type getValue() const { return activeValue; }
	void setValue(const Type &val) { cachedValue = val; }

	bool dirty() const { return activeValue != cachedValue; }

	bool commit() {
		if (!dirty()) {
			return false;
		}
		qDebug() << "settings:" << settings.organizationName() + "." + settings.applicationName() + "." + field << "=" << activeValue << "->"
				 << cachedValue;
		activeValue = cachedValue;
		settings.setValue(field, activeValue);
		emit changed(activeValue);
		return true;
	}

	void clear() {
		cachedValue = defaultValue;
		if (!commit()) {
			emit changed(activeValue);
		}
	}

	virtual void changed(Type) = 0;
};

class SettingsBool : public QObject, public SettingsBase<bool> {
	SETTINGS_BASE(SettingsBool, bool)
	Q_OBJECT

  public:
	using SettingsBase::SettingsBase;
	void connectAction(class QAction *action, bool autoCommit = false);
	void connectCheckBox(class QCheckBox *input, bool autoCommit = false);
	void connectWidgetEnabled(std::initializer_list<class QWidget *> const &targets, bool enableValue = true);
	void depend(SettingsBool &other);
};

class SettingsUInt : public QObject, public SettingsBase<uint> {
	SETTINGS_BASE(SettingsUInt, uint)
	Q_OBJECT

  public:
	using SettingsBase::SettingsBase;
	void connectSpinBox(class QSpinBox *input, bool autoCommit = false);
};

class SettingsSelection : public QObject, public SettingsBase<uint> {
	SETTINGS_BASE(SettingsSelection, uint)
	Q_OBJECT

	const QMap<uint, QString> mapper = {};

  public:
	explicit SettingsSelection(const QString &category, const QString &field, const uint defaultValue, QMap<uint, QString> mapper)
		: SettingsBase(category, field, defaultValue), mapper(mapper) {
		Q_ASSERT(mapper.contains(defaultValue));
		if (settings.value(field + "_count").toUInt() != mapper.size()) {
			clear();
			settings.setValue(field + "_count", mapper.size());
		}
		if (!mapper.contains(getValue())) {
			clear();
		}
	}

	void connectCombobox(class QComboBox *input, bool autoCommit = false);
};
