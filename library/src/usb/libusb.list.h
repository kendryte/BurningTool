#pragma once

#include <stdint.h>
#include <libusb.h>

libusb_device *get_usb_device(struct libusb_context *libusb, uint16_t vid, uint16_t pid, const uint8_t *path);
void free_got_usb_device(libusb_device *dev);
kburn_err_t init_list_all_usb_devices(KBCTX scope);
ssize_t list_usb_ports(KBCTX scope, struct kburnUsbDeviceInfoSlice *list, size_t max_size);
