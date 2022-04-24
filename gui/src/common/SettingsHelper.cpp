#include "SettingsHelper.h"
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QMap>
#include <QSpinBox>

QList<ISettingsBase *> settingsRegistry;

void SettingsBool::connectAction(QAction *action) {
	auto v = getValue();
	action->setChecked(v);
	emit changed(v);
	action->connect(action, &QAction::toggled, this, &SettingsBool::setValue);
	connect(this, &SettingsBool::changed, action, &QAction::setChecked);
}

void SettingsBool::connectCheckBox(QCheckBox *input) {
	auto v = getValue();
	input->setChecked(v);
	emit changed(v);
	input->connect(input, &QCheckBox::toggled, this, &SettingsBool::setValue);
	connect(this, &SettingsBool::changed, input, &QCheckBox::setChecked);
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
	connect(this, &SettingsUInt::changed, input, &QSpinBox::setValue);
}

void SettingsSelection::connectCombobox(QComboBox *input) {
	for (auto k : mapper.keys()) {
		input->addItem(mapper.value(k), k);
	}

	auto v = getValue();
	input->setCurrentText(mapper.value(v));
	emit changed(v);
	input->connect(input, &QComboBox::currentIndexChanged, this, [=]() {
		auto v = input->currentData().toUInt();
		Q_ASSERT(mapper.contains(v));
		setValue(v);
	});
}

QList<ISettingsBase *> ISettingsBase::settingsRegistry;
void ISettingsBase::resetEverythingToDefaults() {
	for (auto element : settingsRegistry) {
		element->clear();
	}
}

void ISettingsBase::commitAllSettings() {
	for (auto element : settingsRegistry) {
		element->commit();
	}
}
