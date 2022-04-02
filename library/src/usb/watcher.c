#include "usb.h"
#include <pthread.h>
#include "basic/event-queue.h"

bool libUsbHasWathcer = false;

struct passing_data
{
	struct libusb_context *ctx;
	struct libusb_device *dev;
	libusb_hotplug_event event;
	void *user_data;
};

static int on_event_sync(struct libusb_context *UNUSED(ctx), struct libusb_device *dev,
						 libusb_hotplug_event event, void *user_data)
{
	KBCTX scope = user_data;
	uint16_t vid, pid;
	uint8_t path[MAX_PATH_LENGTH] = {0};

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
	{
		debug_print(KBURN_LOG_INFO, "libusb event: ARRIVED");
		int ret = usb_get_vid_pid_path(dev, &vid, &pid, path);
		if (ret < LIBUSB_SUCCESS)
			return 0;

		if (usb_device_find(scope, vid, pid, path) != NULL)
		{
			debug_print(KBURN_LOG_DEBUG, "port already open. (this may be issue)");
			return 0;
		}

		kburn_err_t r = open_single_usb_port(scope, dev, NULL);
		if (r != KBurnNoErr)
			debug_print(KBURN_LOG_ERROR, "failed open single port: %ld", r);
	}
	else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event)
	{
		debug_print(KBURN_LOG_INFO, "libusb event: LEFT");
		int ret = usb_get_vid_pid_path(dev, &vid, &pid, path);
		if (ret < LIBUSB_SUCCESS)
			return 0;

		kburnDeviceNode *node = usb_device_find(scope, vid, pid, path);
		if (node != NULL)
			destroy_usb_port(node->disposable_list, node->usb);
	}
	else
	{
		debug_print(KBURN_LOG_ERROR, "Unhandled event %d\n", event);
	}
	return 0;
}

static int on_event_threaded(struct libusb_context *ctx, struct libusb_device *dev,
							 libusb_hotplug_event event, void *user_data)
{
	KBCTX scope = user_data;

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
	{
		debug_print(KBURN_LOG_DEBUG, "libusb event: " COLOR_FMT("ARRIVED"), COLOR_ARG(GREEN));
	}
	else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event)
	{
		debug_print(KBURN_LOG_DEBUG, "libusb event: " COLOR_FMT("LEFT"), COLOR_ARG(YELLOW));
	}
	else
	{
		debug_print(KBURN_LOG_WARN, "Unhandled event %d\n", event);
	}

	struct passing_data *data = calloc(1, sizeof(struct passing_data));
	if (data == NULL)
	{
		debug_print(KBURN_LOG_ERROR, "memory error in libusb event thread");
		return 0;
	}
	data->ctx = ctx;
	data->dev = dev;
	data->event = event;
	data->user_data = user_data;
	queue_push(scope->usb->queue, data);

	return 0;
}

static void thread_event_process(KBCTX scope, const bool *const quit)
{
	init_list_all_usb_devices(scope);

	struct passing_data *recv;
	while (!*quit)
	{
		recv = queue_shift(scope->usb->queue);

		if (recv == NULL)
		{
			do_sleep(1000);
			continue;
		}

		debug_print(KBURN_LOG_DEBUG, "handle event in thread.");
		on_event_sync(recv->ctx, recv->dev, recv->event, recv->user_data);

		free(recv);
	}
}

void usb_monitor_destroy(KBCTX scope)
{
	if (scope->usb->queue)
	{
		queue_destroy(scope->usb->queue);
		scope->usb->queue = NULL;
	}
}

kburn_err_t usb_monitor_prepare(KBCTX scope)
{
	if (!scope->usb->libusb)
		usb_subsystem_init(scope);

	debug_trace_function();
	if (scope->usb->monitor_prepared)
	{
		debug_print(KBURN_LOG_DEBUG, "\talready inited.");
		return KBurnNoErr;
	}

	scope->usb->monitor_prepared = true;

	libUsbHasWathcer = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG);
	debug_print(KBURN_LOG_DEBUG, "libUsbHasWathcer: %d", libUsbHasWathcer);

	int r = libusb_set_option(scope->usb->libusb, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
	if (r < 0)
		debug_print_libusb_error("log level set failed", r);

	if (libUsbHasWathcer)
	{
		kburn_err_t err;

		err = queue_create(&scope->usb->queue);
		if (err != KBurnNoErr)
			return err;

		err = thread_create("my usb handle", thread_event_process, scope, &scope->usb->event_thread);
		if (err != KBurnNoErr)
			return err;
	}
	else
	{
		m_abort("TODO: polling");
		init_list_all_usb_devices(scope);
	}

	return KBurnNoErr;
}

void usb_monitor_pause(KBCTX scope)
{
	debug_trace_function();
	if (scope->usb->monitor_enabled)
	{
		libusb_hotplug_deregister_callback(scope->usb->libusb, scope->usb->monitor_handle);
		scope->usb->monitor_enabled = false;
		debug_print(KBURN_LOG_INFO, "USB monitor disabled");
	}
}

kburn_err_t usb_monitor_resume(KBCTX scope)
{
	debug_trace_function();
	if (scope->usb->monitor_enabled)
		return KBurnNoErr;

	if (!scope->usb->monitor_prepared)
	{
		kburn_err_t r = usb_monitor_prepare(scope);
		if (r != KBurnNoErr)
			return r;
	}

	debug_print(KBURN_LOG_INFO, "\tlibusb_hotplug_register_callback: [%04x:%04x] libUsbHasWathcer=%d", scope->usb->filter.vid, scope->usb->filter.pid, libUsbHasWathcer);
	int ret = libusb_hotplug_register_callback(
		scope->usb->libusb,
		LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
		0,
		scope->usb->filter.vid == KBURN_VIDPID_FILTER_ANY ? LIBUSB_HOTPLUG_MATCH_ANY : scope->usb->filter.vid,
		scope->usb->filter.pid == KBURN_VIDPID_FILTER_ANY ? LIBUSB_HOTPLUG_MATCH_ANY : scope->usb->filter.pid,
		LIBUSB_HOTPLUG_MATCH_ANY,
		libUsbHasWathcer ? on_event_threaded : on_event_sync,
		scope,
		&scope->usb->monitor_handle);

	if (LIBUSB_SUCCESS != ret)
	{
		debug_print_libusb_error("error creating a hotplug callback", ret);
	}

	return make_error_code(KBURN_ERROR_KIND_USB, ret);
}
