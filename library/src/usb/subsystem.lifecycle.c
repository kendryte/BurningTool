#include "usb.h"

void usb_subsystem_deinit(KBCTX scope)
{
	debug_print("deinit_libusb()");

	if (!scope->usb->inited)
		return;
	scope->usb->inited = false;

	libusb_exit(scope->usb->libusb);
}

kburn_err_t usb_subsystem_init(KBCTX scope)
{
	debug_print("usb_subsystem_init()");
	if (scope->usb->inited)
	{
		debug_print("  - already inited");
		return KBurnNoErr;
	}

	const struct libusb_version *version;
	version = libusb_get_version();
	debug_print("using libusb v%d.%d.%d.%d\n\n", version->major, version->minor, version->micro, version->nano);

	int r = libusb_init(&scope->usb->libusb);
	if (r < 0)
	{
		debug_print("libusb init failed: %s\n", libusb_strerror((enum libusb_error)r));
		return r | KBURN_ERROR_KIND_USB;
	}

	scope->usb->inited = true;

	extern bool libUsbHasWathcer;
	libUsbHasWathcer = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG);

	if (libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO) < 0)
		debug_print("log level set failed: %s\n", libusb_strerror((enum libusb_error)r));

	return init_list_all_usb_devices(scope);
}
