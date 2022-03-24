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
	struct libusb_device_descriptor desc;
	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("libusb_get_device_descriptor()", r);
		return r;
	}

	memset(output, 0, MAX_SERIAL_LENGTH);

	if (desc.iSerialNumber == 0)
		return LIBUSB_SUCCESS;

	r = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, output, MAX_SERIAL_LENGTH);
	if (r < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("libusb_get_string_descriptor_ascii()", r);
		return r;
	}

	return LIBUSB_SUCCESS;
}
