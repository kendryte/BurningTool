#pragma once

#include "context.h"
#include "basic/disposable.h"

kburn_err_t on_serial_device_attach(KBCTX scope, const char *path, bool need_verify);
kburn_err_t serial_port_init(kburnSerialDeviceNode *serial, const char *path);
DECALRE_DISPOSE_HEADER(destroy_serial_port, kburnDeviceNode);
