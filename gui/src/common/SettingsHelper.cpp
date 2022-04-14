#include "SettingsHelper.h"
#include <QAction>

SettingsBool::SettingsBool(const QString &category, const QString &field, const bool defaultValue)
	: settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, category), field(field), defaultValue(defaultValue) {
}

bool SettingsBool::getValue() const {
	return settings.value(field, defaultValue).toBool();
}

void SettingsBool::setValue(bool val) {
	emit changed(val);
	return settings.setValue(field, val);
}

void SettingsBool::connectAction(class QAction *action) {
	action->setChecked(getValue());
	emit changed(getValue());
	action->connect(action, &QAction::toggled, this, &SettingsBool::setValue);
}
