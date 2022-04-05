#pragma once

#include "context.h"

kburn_err_t usb_device_hello(kburnDeviceNode *node);
kburn_err_t usb_device_serial_print(kburnDeviceNode *node, const uint8_t *buff, size_t buff_size);
