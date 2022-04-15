#include "AppGlobalSetting.h"
#include <QList>
#include <QSettings>

namespace GlobalSetting {
QList<QString> knownCategory;

SettingsBool autoConfirm{"global", "auto-confirm", false};
SettingsBool autoConfirmManualJob{"global", "auto-confirm-manual", true};
SettingsBool autoConfirmEvenError{"global", "auto-confirm-always", false};
SettingsBool disableUpdate{"global", "no-check-update", false};
SettingsUInt watchVid{"global", "watch-serial-vid", 0x1a86};
SettingsUInt watchPid{"global", "watch-serial-pid", 0x7523};
SettingsUInt appBurnThread{"global", "max-burn-thread", 30};
SettingsUInt usbLedPin{"global", "led-pin", 122};
SettingsUInt usbLedLevel{"global", "led-level", 24};

void restoreDefaults() {
	for (auto category : knownCategory) {
		QSettings settings(QSettings::Scope::UserScope, SETTINGS_CATEGORY, category);
		settings.clear();
	}
}

} // namespace GlobalSetting
