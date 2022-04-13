#pragma once

#include <stdint.h>
#include <libusb.h>

kburn_err_t init_list_all_usb_devices(KBCTX scope);
ssize_t list_usb_ports(struct libusb_context *libusb, struct kburnUsbDeviceInfoSlice *list, size_t max_size);
