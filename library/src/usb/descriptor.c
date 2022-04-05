#include "descriptor.h"
#include "basic/errors.h"
#include "basic/sleep.h"
#include "context.h"
#include "device.h"
#include "private-types.h"
#include <libusb.h>
#include <stdio.h>

kburn_err_t usb_get_vid_pid_path(struct libusb_device *dev, uint16_t *out_vid, uint16_t *out_pid, uint8_t out_path[MAX_PATH_LENGTH]) {
	struct libusb_device_descriptor desc;

	int r = libusb_get_device_descriptor(dev, &desc);
	if (!check_libusb(r)) {
		return make_error_code(KBURN_ERROR_KIND_USB, r);
	}

	*out_vid = desc.idVendor;
	*out_pid = desc.idProduct;

	return usb_get_device_path(dev, out_path);
}

kburn_err_t usb_get_device_path(struct libusb_device *dev, uint8_t path[MAX_PATH_LENGTH]) {
	memset(path, 0, MAX_PATH_LENGTH);

	IfErrorReturn(libusb_get_port_numbers(dev, path, MAX_PATH_LENGTH - 1));

	return KBurnNoErr;
}

kburn_err_t usb_get_field(kburnDeviceNode *node, uint8_t type, uint8_t *output) {
	debug_trace_function();

	if (!node->usb->deviceInfo.descriptor) {
		node->usb->deviceInfo.descriptor = MyAlloc(struct libusb_device_descriptor);
		IfUsbErrorLogReturn(libusb_get_device_descriptor(node->usb->device, node->usb->deviceInfo.descriptor));
	}

	if (type == 0) {
		debug_print(KBURN_LOG_WARN, "  - device descriptor do not have type %d.", type);
		return KBurnNoErr;
	}

	int try = 3, r = 0;
	while (try-- > 0) {
		r = libusb_get_string_descriptor_ascii(node->usb->handle, type, output, 256);
		if (r != LIBUSB_ERROR_BUSY)
			break;

		debug_print_libusb_error("  %d> libusb_get_string_descriptor_ascii()", r, try);
		do_sleep(1000);
	}

	IfUsbErrorLogReturn(r);

	return KBurnNoErr;
}

const char *usb_debug_path_string(const uint8_t *path) {
	static char debug[MAX_PATH_LENGTH * 3 + 1] = "";
	char *debug_itr = debug;
	for (int i = 0; i < MAX_PATH_LENGTH - 1; i++) {
		debug_itr += sprintf(debug_itr, "%02x:", path[i]);
	}
	*(debug_itr - 1) = '\0';
	return debug;
}
