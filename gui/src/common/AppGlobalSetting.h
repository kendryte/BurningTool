#pragma once

#include "SettingsHelper.h"

namespace GlobalSetting {

extern SettingsBool autoConfirm;
extern SettingsUInt autoConfirmTimeout;
extern SettingsBool autoConfirmManualJob;
extern SettingsUInt autoConfirmManualJobTimeout;
extern SettingsBool autoConfirmEvenError;
extern SettingsUInt autoConfirmEvenErrorTimeout;
extern SettingsBool disableUpdate;
extern SettingsUInt watchVid;
extern SettingsUInt watchPid;
extern SettingsUInt appBurnThread;
extern SettingsUInt usbLedPin;
extern SettingsUInt usbLedLevel;
extern SettingsSelection flashTarget;

void restoreDefaults();

} // namespace GlobalSetting
