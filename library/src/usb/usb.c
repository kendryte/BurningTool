#include "usb.h"

int usb_get_vid_pid_path(struct libusb_device *dev, uint16_t *out_vid, uint16_t *out_pid, uint8_t *out_path)
{
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("libusb_get_device_descriptor()", r);
		return r;
	}

	*out_vid = desc.idVendor;
	*out_pid = desc.idProduct;

	return usb_get_device_path(dev, out_path);
}

int usb_get_device_path(struct libusb_device *dev, uint8_t *path)
{
	int r;

	r = libusb_get_port_numbers(dev, path, MAX_PATH_LENGTH - 1);
	if (r < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("usb_get_device_path: libusb_get_port_numbers()", r);
		return r;
	}

	return LIBUSB_SUCCESS;
}

int usb_get_device_serial(libusb_device *dev, libusb_device_handle *handle, uint8_t *output)
{
	debug_print("usb_get_device_serial()");
	struct libusb_device_descriptor desc;
	CheckLibusbError(
		libusb_get_device_descriptor(dev, &desc));

	memset(output, 0, MAX_SERIAL_LENGTH);

	if (desc.iSerialNumber == 0)
	{
		debug_print("  - device do not have serial.");
		return LIBUSB_SUCCESS;
	}

	int try = 3, r = 0;
	while (try-- > 0)
	{
		r = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, output, MAX_SERIAL_LENGTH);
		if (r != LIBUSB_ERROR_BUSY)
			break;

		debug_print_libusb_error("  %d> libusb_get_string_descriptor_ascii()", r, try);
		do_sleep(1000);
	}

	if (r >= LIBUSB_SUCCESS)
		debug_print("serial: %s", output);
	else if (r == LIBUSB_ERROR_BUSY)
		debug_print("abort try to get serial number");
	else
		debug_print_libusb_error("  - libusb_get_string_descriptor_ascii()", r);

	return r;
}
