#pragma once

#include "basic/disposable.h"
#include "context.h"

kburn_err_t on_serial_device_attach(KBCTX scope, const char *path);
kburn_err_t serial_port_init(kburnSerialDeviceNode *serial, const char *path);
DECALRE_DISPOSE_HEADER(destroy_serial_port, kburnDeviceNode);
