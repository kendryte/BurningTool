#include "SettingsHelper.h"
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QMap>
#include <QSpinBox>

QList<ISettingsBase *> settingsRegistry;

void SettingsBool::connectAction(QAction *action, bool autoCommit) {
	auto v = getValue();
	action->setChecked(v);
	emit changed(v);
	action->connect(action, &QAction::toggled, this, autoCommit ? &SettingsBool::setValueImmediate : &SettingsBool::setValue);
	connect(this, &SettingsBool::changed, action, &QAction::setChecked);
}

void SettingsBool::connectCheckBox(QCheckBox *input, bool autoCommit) {
	auto v = getValue();
	input->setChecked(v);
	input->connect(input, &QCheckBox::toggled, this, autoCommit ? &SettingsBool::setValueImmediate : &SettingsBool::setValue);
	connect(this, &SettingsBool::changed, input, &QCheckBox::setChecked);
}

void SettingsBool::connectWidgetEnabled(std::initializer_list<QWidget *> const &targets, bool enableValue) {
	for (QWidget *target : targets) {
		target->setEnabled(getValue() == enableValue);
		connect(this, &SettingsBool::changed, target, [=]() { target->setEnabled(getValue() == enableValue); });
	}
}

void SettingsBool::depend(SettingsBool &other) {
	if (!other.getValue()) {
		if (getValue()) {
			setValueImmediate(false);
		}
	}
	connect(&other, &SettingsBool::changed, this, [=](bool v) {
		if (!v) {
			setValueImmediate(false);
		}
	});
}

void SettingsUInt::connectSpinBox(QSpinBox *input, bool autoCommit) {
	auto v = getValue();
	input->setValue(v);
	if (input->value() != v) {
		v = input->value();
		setValue(v);
	}
	input->connect(input, &QSpinBox::valueChanged, this, autoCommit ? &SettingsUInt::setValueImmediate : &SettingsUInt::setValue);
	connect(this, &SettingsUInt::changed, input, &QSpinBox::setValue);
}

void SettingsSelection::connectCombobox(QComboBox *input, bool autoCommit) {
	input->clear();
	for (auto k : mapper.keys()) {
		input->addItem(mapper.value(k), k);
	}

	auto v = getValue();
	input->setCurrentText(mapper.value(v));
	input->connect(input, &QComboBox::currentIndexChanged, this, [=]() {
		auto v = input->currentData().toUInt();
		Q_ASSERT(mapper.contains(v));
		if (autoCommit) {
			setValueImmediate(v);
		} else {
			setValue(v);
		}
	});
	connect(this, &SettingsSelection::changed, input, [=](uint value) {
		Q_ASSERT(mapper.contains(value));
		input->setCurrentText(mapper.value(value));
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
