#pragma once

#include "context.h"
#include <sys/types.h>

kburn_err_t init_list_all_serial_devices(KBCTX scope);
ssize_t list_serial_ports(KBCTX scope, struct kburnSerialDeviceInfoSlice *list, size_t max_size);
