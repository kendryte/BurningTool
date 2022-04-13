#include "monitor.h"
#include "base.h"
#include "context.h"
#include "descriptor.h"
#include "device.h"
#include "libusb.list.h"
#include "lifecycle.h"
#include "private-types.h"
#include "subsystem.h"
#include "components/call-user-handler.h"
#include "components/device-link-list.h"
#include "components/queued-thread.h"
#include <libusb.h>
#include <pthread.h>

static void init_list_all_usb_devices_threaded(void *UNUSED(_ctx), KBCTX scope, const bool *const q) {
	if (!q) {
		init_list_all_usb_devices(scope);
	}
}

void _auto_libusb_free_device_list(libusb_device ***pval) {
	if (*pval == NULL)
		return;
	libusb_free_device_list(*pval, true);
}

static bool open_this_usb_device(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t in_path[MAX_USB_PATH_LENGTH]) {
	debug_trace_function("%d, %d, %.8s", vid, pid, usb_debug_path_string(in_path));
	libusb_device **__attribute__((cleanup(_auto_libusb_free_device_list))) list = NULL;
	ssize_t dev_count = libusb_get_device_list(scope->usb->libusb, &list);
	if (!check_libusb(dev_count)) {
		debug_print_libusb_error("libusb_get_device_list()", dev_count);
		return NULL;
	}
	for (int i = 0; i < dev_count; i++) {
		libusb_device *dev = list[i];
		struct libusb_device_descriptor desc;

		int r = libusb_get_device_descriptor(dev, &desc);
		if (!check_libusb(r)) {
			continue;
		}

		if (vid != desc.idVendor || pid != desc.idProduct) {
			continue;
		}

		uint8_t path[MAX_USB_PATH_LENGTH] = {0};
		if (usb_get_device_path(dev, path) < LIBUSB_SUCCESS) {
			continue;
		}

		if (strncmp((const char *)path, (const char *)in_path, MAX_USB_PATH_LENGTH) == 0) {
			kburn_err_t r = open_single_usb_port(scope, dev, true, NULL);
			if (r != KBurnNoErr) {
				debug_print(KBURN_LOG_ERROR, "failed open single port: %" PRIu64, r);
				return false;
			}
			return true;
		}
	}

	debug_print(KBURN_LOG_ERROR, "failed get device!");
	return false;
}

static void _pump_libusb_event(struct passing_data *recv) {
	debug_print(KBURN_LOG_DEBUG, "handle event in thread.");

	KBCTX scope = ((struct passing_data *)recv)->scope;
	CALL_HANDLE_SYNC(scope->on_list_change, true);

	libusb_hotplug_event event = ((struct passing_data *)recv)->event;
	kburnUsbDeviceInfoSlice defInfo = ((struct passing_data *)recv)->dev;

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
		debug_print(KBURN_LOG_INFO, "libusb event: ARRIVED");

		if (get_device_by_usb_port_path(scope, defInfo.idVendor, defInfo.idProduct, defInfo.path) != NULL) {
			debug_print(KBURN_LOG_DEBUG, "port already open. (this may be issue)");
			return;
		}

		open_this_usb_device(scope, defInfo.idVendor, defInfo.idProduct, defInfo.path);

	} else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
		debug_print(KBURN_LOG_INFO, "libusb event: LEFT");
		kburnDeviceNode *node = get_device_by_usb_port_path(scope, defInfo.idVendor, defInfo.idProduct, defInfo.path);
		if (node != NULL) {
			destroy_usb_port(node->disposable_list, node->usb);
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
	event_thread_queue(scope->usb->event_queue, data, false);
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

	IfErrorReturn(event_thread_init(scope, "libusb pump", pump_libusb_event, &scope->usb->event_queue));

	if (scope->usb->event_mode == USB_EVENT_CALLBACK) {
		IfErrorReturn(usb_monitor_callback_prepare(scope));
	} else {
		IfErrorReturn(usb_monitor_polling_prepare(scope));
	}

	thread_create("usb init scan", init_list_all_usb_devices_threaded, NULL, scope, NULL);

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
