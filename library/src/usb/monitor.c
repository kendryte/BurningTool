#include "monitor.h"
#include "base.h"
#include "context.h"
#include "descriptor.h"
#include "libusb.list.h"
#include "lifecycle.h"
#include "private-types.h"
#include "subsystem.h"
#include "components/device-link-list.h"
#include "components/queued-thread.h"
#include <pthread.h>

static void init_list_all_usb_devices_threaded(void *UNUSED(_ctx), KBCTX scope, const bool *const q) {
	if (!q) {
		init_list_all_usb_devices(scope);
	}
}

static void _pump_libusb_event(struct passing_data *recv) {
	debug_print(KBURN_LOG_DEBUG, "handle event in thread.");

	libusb_hotplug_event event = ((struct passing_data *)recv)->event;
	kburnUsbDeviceInfoSlice defInfo = ((struct passing_data *)recv)->dev;
	KBCTX scope = ((struct passing_data *)recv)->scope;

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
		debug_print(KBURN_LOG_INFO, "libusb event: ARRIVED");

		if (get_device_by_usb_port_path(scope, defInfo.idVendor, defInfo.idProduct, defInfo.path) != NULL) {
			debug_print(KBURN_LOG_DEBUG, "port already open. (this may be issue)");
			return;
		}

		libusb_device *dev = get_usb_device(scope->usb->libusb, defInfo.idVendor, defInfo.idProduct, defInfo.path);
		if (!dev) {
			debug_print(
				KBURN_LOG_ERROR, "failed get device: [%04x: %04x] %s", defInfo.idVendor, defInfo.idProduct, usb_debug_path_string(defInfo.path));
			return;
		}
		kburn_err_t r = open_single_usb_port(scope, dev, true, NULL);
		if (r != KBurnNoErr) {
			debug_print(KBURN_LOG_ERROR, "failed open single port: %" PRIu64, r);
		}
	} else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
		debug_print(KBURN_LOG_INFO, "libusb event: LEFT");
		kburnDeviceNode *node = get_device_by_usb_port_path(scope, defInfo.idVendor, defInfo.idProduct, defInfo.path);
		if (node != NULL) {
			destroy_usb_port(node->disposable_list, node->usb);
		} else {
			if (scope->on_disconnect.handler) {
				debug_print(KBURN_LOG_DEBUG, "\tscope::on_disconnect()");
				scope->on_disconnect.handler(scope->on_disconnect.context, NULL);
			}
		}
	} else {
		debug_print(KBURN_LOG_ERROR, "Unhandled event %d\n", event);
	}
}

void pump_libusb_event(KBCTX UNUSED(scope), void *recv) {
	_pump_libusb_event(recv);
	free(recv);
}

void push_libusb_event(KBCTX scope, libusb_hotplug_event event, const struct kburnUsbDeviceInfoSlice *devInfo) {
	struct passing_data *data = calloc(1, sizeof(struct passing_data));
	if (data == NULL) {
		debug_print(KBURN_LOG_ERROR, "memory error in libusb event thread");
	}
	data->scope = scope;
	data->dev = *devInfo;
	data->event = event;
	event_thread_queue(scope->usb->event_queue, data);
}

void usb_monitor_destroy(KBCTX scope) {
	if (!scope->usb->monitor_prepared) {
		return;
	}
	scope->usb->monitor_prepared = false;

	if (scope->usb->event_queue) {
		event_thread_deinit(scope, &scope->usb->event_queue);
		scope->usb->event_queue = NULL;
	}

	if (scope->usb->event_mode == USB_EVENT_CALLBACK) {
		usb_monitor_callback_destroy(scope);
	} else {
		usb_monitor_polling_destroy(scope);
	}
}

kburn_err_t usb_monitor_prepare(KBCTX scope) {
	if (!scope->usb->libusb) {
		usb_subsystem_init(scope);
	}

	debug_trace_function();
	if (scope->usb->monitor_prepared) {
		debug_print(KBURN_LOG_DEBUG, "\talready inited.");
		return KBurnNoErr;
	}

	scope->usb->monitor_prepared = true;

	if (libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
		debug_print(KBURN_LOG_DEBUG, "libUsbHasWathcer: callback");
		scope->usb->event_mode = USB_EVENT_CALLBACK;
	} else {
		debug_print(KBURN_LOG_DEBUG, "libUsbHasWathcer: polling");
		scope->usb->event_mode = USB_EVENT_POLLING;
	}

	int r = libusb_set_option(scope->usb->libusb, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
	if (r < 0) {
		debug_print_libusb_error("log level set failed", r);
	}

	IfErrorReturn(event_thread_init(scope, "libusb pump", pump_libusb_event, &scope->usb->event_queue));

	if (scope->usb->event_mode == USB_EVENT_CALLBACK) {
		IfErrorReturn(usb_monitor_callback_prepare(scope));
	} else {
		IfErrorReturn(usb_monitor_polling_prepare(scope));
	}

	kbthread no_use;
	thread_create("usb init scan", init_list_all_usb_devices_threaded, NULL, scope, &no_use);

	return KBurnNoErr;
}

void usb_monitor_pause(KBCTX scope) {
	debug_trace_function();
	if (scope->usb->monitor_enabled) {
		if (scope->usb->event_mode == USB_EVENT_CALLBACK) {
			usb_monitor_callback_pause(scope);
		} else {
			usb_monitor_polling_pause(scope);
		}

		scope->usb->monitor_enabled = false;
		debug_print(KBURN_LOG_INFO, "USB monitor disabled");
	}
}

kburn_err_t usb_monitor_resume(KBCTX scope) {
	debug_trace_function();
	if (scope->usb->monitor_enabled) {
		return KBurnNoErr;
	}

	if (!scope->usb->monitor_prepared) {
		kburn_err_t r = usb_monitor_prepare(scope);
		if (r != KBurnNoErr) {
			return r;
		}
	}

	if (scope->usb->event_mode == USB_EVENT_CALLBACK) {
		IfErrorReturn(usb_monitor_callback_resume(scope));
	} else {
		IfErrorReturn(usb_monitor_polling_resume(scope));
	}

	return KBurnNoErr;
}
