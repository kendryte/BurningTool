#include "subsystem.h"
#include "base.h"
#include "basic/disposable.h"
#include "basic/errors.h"
#include "basic/resource-tracker.h"
#include "monitor.h"
#include "private-types.h"
#include <libusb.h>
#include <pthread.h>

static void thread_libusb_handle_events(void *UNUSED(ctx), KBCTX scope, const bool *const quit) {
	struct timeval timeout = {
		.tv_sec = 1,
		.tv_usec = 0,
	};
	while (!*quit) {
		libusb_handle_events_timeout(scope->usb->libusb, &timeout);
	}
}

void usb_subsystem_deinit(KBCTX scope) {
	debug_trace_function();
	if (!scope->usb->subsystem_inited) {
		debug_print(KBURN_LOG_DEBUG, "  - already deinited");
		return;
	}

	usb_monitor_destroy(scope);

	if (scope->usb->libusb) {
		debug_print(KBURN_LOG_INFO, "libusb_exit!");
		libusb_exit(scope->usb->libusb);
		scope->usb->libusb = NULL;
	}

	scope->usb->subsystem_inited = false;
	debug_trace_function("DONE");
}

kburn_err_t usb_subsystem_init(KBCTX scope) {
	DeferEnabled;

	debug_trace_function();
	if (scope->usb->subsystem_inited) {
		debug_print(KBURN_LOG_DEBUG, "  - already inited");
		return KBurnNoErr;
	}

	scope->usb->detach_kernel_driver = libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER);
	debug_print(KBURN_LOG_INFO, "libusb can detach kernel driver: %d", scope->usb->detach_kernel_driver);

	const struct libusb_version *version = libusb_get_version();
	debug_print(KBURN_LOG_INFO, "\tlibusb v%d.%d.%d.%d\n\n", version->major, version->minor, version->micro, version->nano);

	int r = libusb_init(&scope->usb->libusb);
	if (r < 0) {
		debug_print_libusb_error("libusb init failed", r);
		return make_error_code(KBURN_ERROR_KIND_USB, r);
	}
	DeferCall(libusb_exit, scope->usb->libusb);

	kburn_err_t err = thread_create("my libusb evt", thread_libusb_handle_events, NULL, scope, &scope->usb->libusb_thread);
	if (err != KBurnNoErr) {
		return err;
	}
	thread_resume(scope->usb->libusb_thread);

	debug_print(KBURN_LOG_DEBUG, "libusb init complete");

	scope->usb->subsystem_inited = true;
	DeferAbort;
	return KBurnNoErr;
}
