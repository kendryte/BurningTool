#include "debug/print.h"
#include "types.h"
#include <initguid.h>
#include <Setupapi.h>
#include <windows.h>

void driver_get_devinfo_free(kburnSerialDeviceInfo deviceInfo) { free(deviceInfo.path); }

kburnSerialDeviceInfo driver_get_devinfo(const char *path) { return (kburnSerialDeviceInfo){.isTTY = false, .path = strdup(path)}; }
