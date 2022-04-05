#pragma once

#include "canaan-burn/canaan-burn.h"
#include <stdint.h>

kburn_err_t usb_get_vid_pid_path(struct libusb_device *dev, uint16_t *out_vid, uint16_t *out_pid, uint8_t out_path[MAX_PATH_LENGTH]);
kburn_err_t usb_get_device_path(struct libusb_device *dev, uint8_t path[MAX_PATH_LENGTH]);
kburn_err_t usb_get_field(kburnDeviceNode *node, uint8_t type, uint8_t *output); // output at least 256 bytes
const char *usb_debug_path_string(const uint8_t *path);
