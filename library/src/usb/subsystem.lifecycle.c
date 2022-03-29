#include "usb.h"
#include <pthread.h>

static void thread_libusb_handle_events(KBCTX scope, const bool *const quit)
{
	struct timeval timeout = {
		.tv_sec = 1,
		.tv_usec = 0,
	};
	while (!*quit)
	{
		libusb_handle_events_timeout(scope->usb->libusb, &timeout);
	}
}

void usb_subsystem_deinit(KBCTX scope)
{
	debug_print("deinit_libusb()");
	if (!scope->usb->subsystem_inited)
	{
		debug_print("  - already deinited");
		return;
	}

	usb_monitor_destroy(scope);

	if (scope->usb->libusb)
	{
		libusb_exit(scope->usb->libusb);
		scope->usb->libusb = NULL;
	}

	scope->usb->subsystem_inited = false;
}

kburn_err_t usb_subsystem_init(KBCTX scope)
{
	DeferEnabled;

	debug_print("usb_subsystem_init()");
	if (scope->usb->subsystem_inited)
	{
		debug_print("  - already inited");
		return KBurnNoErr;
	}

	scope->usb->detach_kernel_driver = libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER);
	debug_print("libusb can detach kernel driver: %d", scope->usb->detach_kernel_driver);

	const struct libusb_version *version = libusb_get_version();
	debug_print("\tlibusb v%d.%d.%d.%d\n\n", version->major, version->minor, version->micro, version->nano);

	int r = libusb_init(&scope->usb->libusb);
	if (r < 0)
	{
		debug_print_libusb_error("libusb init failed", r);
		return r | KBURN_ERROR_KIND_USB;
	}
	DeferCall(libusb_exit, scope->usb->libusb);

	kburn_err_t err = thread_create("libusb events", thread_libusb_handle_events, scope, &scope->usb->libusb_thread);
	if (err != KBurnNoErr)
		return err;

	debug_print("libusb init complete");

	scope->usb->subsystem_inited = true;
	DeferAbort;
	return KBurnNoErr;
}
