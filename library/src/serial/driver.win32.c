#include "context.h"
#include <windows.h>
#include <initguid.h>
#include "debug/print.h"

// #define TOTAL_BYTES_READ 48
// #define OFFSET_BYTES 48

// https://aticleworld.com/get-com-port-properties-win32-api/
// BOOL ReadComConfiguration(HKEY hKeyParent, PWCHAR subkey, PWCHAR valueName, PWCHAR *readData) {
// 	HKEY hKey;
// 	DWORD len = TOTAL_BYTES_READ;
// 	DWORD readDataLen = len;
// 	PWCHAR readBuffer = (PWCHAR)malloc(sizeof(PWCHAR) * len);
// 	if (readBuffer == NULL) {
// 		return FALSE;
// 	}
// 	// Check if the registry exists
// 	DWORD Ret = RegOpenKeyEx(hKeyParent, subkey, 0, KEY_READ, &hKey);
// 	if (Ret == ERROR_SUCCESS) {
// 		Ret = RegQueryValueEx(hKey, valueName, NULL, NULL, (BYTE *)readBuffer, &readDataLen);
// 		while (Ret == ERROR_MORE_DATA) {
// 			// Get a buffer that is big enough.
// 			len += OFFSET_BYTES;
// 			readBuffer = (PWCHAR)realloc(readBuffer, len);
// 			readDataLen = len;
// 			Ret = RegQueryValueEx(hKey, valueName, NULL, NULL, (BYTE *)readBuffer, &readDataLen);
// 		}
// 		if (Ret != ERROR_SUCCESS) {
// 			// close registry
// 			RegCloseKey(hKey);
// 			return false;
// 		}
// 		// copy read data
// 		*readData = readBuffer;
// 		// close registry
// 		RegCloseKey(hKey);
// 		return true;
// 	} else {
// 		return false;
// 	}
// }

kburnSerialDeviceInfo driver_get_devinfo(const char *path) {
	kburnSerialDeviceInfo ret;
	ret.isTTY = false;
	snprintf(ret.path, MAX_SERIAL_PATH_SIZE, "%s", path);

	return ret;
}
