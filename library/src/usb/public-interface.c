#include "usb.h"

kburn_err_t kburnOpenUSB(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *serial)
{
	if (!scope->usb->libusb)
		usb_subsystem_init(scope);

	libusb_device *dev = get_usb_device(scope->usb->libusb, vid, pid, serial);

	if (dev != NULL)
	{
		return open_single_usb_port(scope, dev);
	}
	return KBurnUsbDeviceNotFound | KBURN_ERROR_KIND_COMMON;
}

kburn_err_t kburnPollUSB(KBCTX scope)
{
	if (!scope->usb->libusb)
		usb_subsystem_init(scope);

	return init_list_all_usb_devices(scope);
}
