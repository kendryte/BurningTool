#include <initguid.h>
#include <windows.h>
#include <Setupapi.h>
#include "debug/print.h"
#include "types.h"

void driver_get_devinfo_free(kburnSerialDeviceInfo deviceInfo)
{
	if (deviceInfo.usbDriver)
		free(deviceInfo.usbDriver);
}

kburnSerialDeviceInfo driver_get_devinfo(const char *UNUSED(path))
{
	return (kburnSerialDeviceInfo){
		.isTTY = false,
	};
}
