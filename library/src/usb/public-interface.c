#include "basic/errors.h"
#include "basic/lock.h"
#include "components/call-user-handler.h"
#include "usb.h"

kburn_err_t kburnOpenUSB(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *path)
{
	if (!scope->usb->libusb)
		usb_subsystem_init(scope);

	libusb_device *dev = get_usb_device(scope->usb->libusb, vid, pid, path);

	if (dev == NULL)
		return make_error_code(KBURN_ERROR_KIND_COMMON, KBurnUsbDeviceNotFound);

	IfErrorReturn(
		open_single_usb_port(scope, dev, NULL));

	return KBurnNoErr;
}

kburn_err_t kburnPollUSB(KBCTX scope)
{
	if (!scope->usb->libusb)
		usb_subsystem_init(scope);

	return init_list_all_usb_devices(scope);
}
