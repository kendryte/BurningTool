#include "context.h"
#include "encoding.h"
#include <windows.h>
#include <initguid.h>
#include <Setupapi.h>
#include "debug/print.h"

kburnSerialDeviceInfo driver_get_devinfo(const char *path) {
	kburnSerialDeviceInfo ret;
	memset(&ret, 0, sizeof(kburnSerialDeviceInfo));
	ret.isTTY = false;
	snprintf(ret.path, MAX_SERIAL_PATH_SIZE, "%s", path);

	HDEVINFO dInfoSet = SetupDiGetClassDevsW(&GUID_DEVINTERFACE_COMPORT, NULL, NULL, DIGCF_DEVICEINTERFACE);
	if (dInfoSet == INVALID_HANDLE_VALUE) {
		debug_print_win32("WIN32 SetupDiGetClassDevs failed.");
		return ret;
	}

	SP_DEVINFO_DATA DeviceInfoData;
	ZeroMemory(&DeviceInfoData, sizeof(SP_DEVINFO_DATA));
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	int DeviceIndex = 0;

	char pathTest[10];
	snprintf(pathTest, sizeof(pathTest), "(%s)", path);

	while (SetupDiEnumDeviceInfo(dInfoSet, DeviceIndex++, &DeviceInfoData)) {
		WCHAR buff[1024];
		if (!SetupDiGetDeviceRegistryPropertyW(dInfoSet, &DeviceInfoData, SPDRP_FRIENDLYNAME, 0, (LPBYTE)buff, 1024, NULL)) {
			debug_print_win32("WIN32 SetupDiGetDeviceRegistryPropertyW failed.");
			continue;
		}
		utf8_encode(buff, ret.title, sizeof(ret.title));
		if (!strstr(ret.title, pathTest)) {
			ret.title[0] = '\0';
			continue;
		}

		if (SetupDiGetDeviceRegistryPropertyW(dInfoSet, &DeviceInfoData, SPDRP_HARDWAREID, 0, (LPBYTE)buff, 1024, NULL)) {
			utf8_encode(buff, ret.hwid, sizeof(ret.hwid));

			if (strstr(ret.hwid, "USB\\")) {
				char *s1 = strstr(ret.hwid, "VID_");
				if (s1) {
					ret.usbIdVendor = (uint16_t)strtol(s1 + 4, NULL, 16);
				}

				char *s2 = strstr(ret.hwid, "PID_");
				if (s2) {
					ret.usbIdProduct = (uint16_t)strtol(s2 + 4, NULL, 16);
				}
			}
		} else {
			debug_print_win32("WIN32 SetupDiGetDeviceRegistryPropertyW failed.");
			ret.hwid[0] = '\0';
		}
		break;
	}

	SetupDiDestroyDeviceInfoList(dInfoSet);
	return ret;
}
