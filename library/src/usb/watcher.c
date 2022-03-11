#include "usb.h"

static libusb_hotplug_callback_handle monitor_handle;
static bool monitor_enabled = false;

static int on_event(struct libusb_context *UNUSED(ctx), struct libusb_device *dev,
					libusb_hotplug_event event, void *UNUSED(user_data))
{
	static libusb_device_handle *dev_handle = NULL;
	struct libusb_device_descriptor desc;
	int rc;

	(void)libusb_get_device_descriptor(dev, &desc);

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
	{
		rc = libusb_open(dev, &dev_handle);
		if (LIBUSB_SUCCESS != rc)
		{
			printf("Could not open USB device\n");
		}
	}
	else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event)
	{
		if (dev_handle)
		{
			libusb_close(dev_handle);
			dev_handle = NULL;
		}
	}
	else
	{
		printf("Unhandled event %d\n", event);
	}

	return 0;
}

void usb_monitor_pause(KBCTX scope)
{
	debug_print("usb_monitor_pause()");
	libusb_hotplug_deregister_callback(scope->usb->libusb, monitor_handle);
	monitor_enabled = false;
}

kburn_err_t usb_monitor_resume(KBCTX scope)
{
	debug_print("usb_monitor_resume()");
	if (scope->usb->monitor_enabled)
		return KBurnNoErr;

	if (!scope->usb->inited)
	{
		kburn_err_t r = usb_subsystem_init(scope);
		if (r != KBurnNoErr)
		{
			debug_print("error creating a hotplug callback: %s", libusb_strerror(r & ~KBURN_ERROR_KIND_USB));
			return r;
		}
	}

	int ret = libusb_hotplug_register_callback(
		scope->usb->libusb,
		LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
		0,
		scope->usb->filter.vid == KBURN_VIDPID_FILTER_ANY ? LIBUSB_HOTPLUG_MATCH_ANY : scope->usb->filter.vid,
		scope->usb->filter.pid == KBURN_VIDPID_FILTER_ANY ? LIBUSB_HOTPLUG_MATCH_ANY : scope->usb->filter.pid,
		LIBUSB_HOTPLUG_MATCH_ANY,
		on_event,
		NULL,
		&monitor_handle);

	if (LIBUSB_SUCCESS != ret)
	{
		debug_print("error creating a hotplug callback: %s", libusb_strerror(ret));
	}

	return KBURN_ERROR_KIND_USB | ret;
}
