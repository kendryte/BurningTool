#include "AppGlobalSetting.h"
#include <canaan-burn/canaan-burn.h>
#include <QList>
#include <QSettings>

namespace GlobalSetting {
namespace Private {

static const QMap<uint, QString> flashTargetMap = {
	{kburnUsbIspCommandTaget::KBURN_USB_ISP_EMMC,   "EMMC"   },
	{kburnUsbIspCommandTaget::KBURN_USB_ISP_OTP,    "OTP"    },
	{kburnUsbIspCommandTaget::KBURN_USB_ISP_NAND,   "NAND"   },
	{kburnUsbIspCommandTaget::KBURN_USB_ISP_SDCARD, "SD Card"},
};
}

SettingsBool autoConfirm{"global", "auto-confirm", true};
SettingsUInt autoConfirmTimeout{"global", "auto-confirm-timeout", 3};
SettingsBool autoConfirmManualJob{"global", "auto-confirm-manual", false};
SettingsUInt autoConfirmManualJobTimeout{"global", "auto-confirm-manual-timeout", 5};
SettingsBool autoConfirmEvenError{"global", "auto-confirm-always", false};
SettingsUInt autoConfirmEvenErrorTimeout{"global", "auto-confirm-always-timeout", 5};
SettingsBool disableUpdate{"global", "no-check-update", false};
SettingsUInt watchVid{"global", "watch-serial-vid", 0x1a86};
SettingsUInt watchPid{"global", "watch-serial-pid", 0x7523};
SettingsUInt appBurnThread{"global", "max-burn-thread", 30};
SettingsUInt usbLedPin{"global", "led-pin", 122};
SettingsUInt usbLedLevel{"global", "led-level", 24};
SettingsSelection flashTarget{"burning", "flash-target", kburnUsbIspCommandTaget::KBURN_USB_ISP_EMMC, Private::flashTargetMap};

} // namespace GlobalSetting
