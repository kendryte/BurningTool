#include "SettingsHelper.h"
#include <QAction>
#include <QCheckBox>
#include <QMap>
#include <QSpinBox>

void SettingsBool::connectAction(QAction *action) {
	auto v = getValue();
	action->setChecked(v);
	emit changed(v);
	action->connect(action, &QAction::toggled, this, &SettingsBool::setValue);
}

void SettingsBool::connectCheckBox(QCheckBox *input) {
	auto v = getValue();
	input->setChecked(v);
	emit changed(v);
	input->connect(input, &QCheckBox::toggled, this, &SettingsBool::setValue);
}

void SettingsUInt::connectSpinBox(QSpinBox *input) {
	auto v = getValue();
	input->setValue(v);
	if (input->value() != v) {
		v = input->value();
		setValue(v);
	}
	emit changed(v);
	input->connect(input, &QSpinBox::valueChanged, this, &SettingsUInt::setValue);
}
