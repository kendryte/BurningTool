#pragma once

#include "types.h"

kburnSerialDeviceInfo driver_get_devinfo(const char *path);
void driver_get_devinfo_free(kburnSerialDeviceInfo);
