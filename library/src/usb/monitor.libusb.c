#include "monitor.h"
#include "context.h"
#include "private-types.h"
#include "canaan-burn/canaan-burn.h"
#include <stdlib.h>
#include <libusb.h>
#include "debug/color.h"
#include "debug/print.h"

static int on_libusb_hotplug_event(struct libusb_context *UNUSED(ctx), struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
	KBCTX scope = user_data;

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
		debug_print(KBURN_LOG_DEBUG, "libusb event: " COLOR_FMT("ARRIVED"), COLOR_ARG(GREEN));
		push_libusb_event(scope, event, dev);
	} else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
		debug_print(KBURN_LOG_DEBUG, "libusb event: " COLOR_FMT("LEFT"), COLOR_ARG(YELLOW));
		push_libusb_event(scope, event, dev);
	} else {
		debug_print(KBURN_LOG_WARN, "Unhandled event %d\n", event);
	}

	return 0;
}

kburn_err_t usb_monitor_callback_prepare(KBCTX UNUSED(scope)) { return KBurnNoErr; }
void usb_monitor_callback_destroy(KBCTX UNUSED(scope)) {}
void usb_monitor_callback_pause(KBCTX scope) { libusb_hotplug_deregister_callback(scope->usb->libusb, scope->usb->event_handle); }
kburn_err_t usb_monitor_callback_resume(KBCTX scope) {
	debug_print(KBURN_LOG_INFO, "\tlibusb_hotplug_register_callback: [%04x:%04x]", scope->usb->filter.vid, scope->usb->filter.pid);

	IfUsbErrorLogReturn(
		libusb_hotplug_register_callback(scope->usb->libusb, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0,
										 scope->usb->filter.vid == KBURN_VIDPID_FILTER_ANY ? LIBUSB_HOTPLUG_MATCH_ANY : scope->usb->filter.vid,
										 scope->usb->filter.pid == KBURN_VIDPID_FILTER_ANY ? LIBUSB_HOTPLUG_MATCH_ANY : scope->usb->filter.pid,
										 LIBUSB_HOTPLUG_MATCH_ANY, on_libusb_hotplug_event, scope, &scope->usb->event_handle));

	return KBurnNoErr;
}
