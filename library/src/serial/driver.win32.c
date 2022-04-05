#include "context.h"
#include "debug/print.h"
#include <initguid.h>
#include <Setupapi.h>
#include <windows.h>

kburnSerialDeviceInfo driver_get_devinfo(const char *path) {
	kburnSerialDeviceInfo ret;
	ret.isTTY = false;
	snprintf(ret.path, MAX_SERIAL_PATH_SIZE, "%s", path);
	return ret;
}
