#include "usb.h"

bool libUsbHasWathcer = false;

static int on_event(struct libusb_context *UNUSED(ctx), struct libusb_device *dev,
					libusb_hotplug_event event, void *user_data)
{
	KBCTX scope = user_data;

	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
	{
		uint8_t serialString[MAX_SERIAL_LENGTH];
		struct usb_get_serial_ret ret = usb_get_serial(dev, serialString, MAX_SERIAL_LENGTH);
		if (ret.error != LIBUSB_SUCCESS)
		{
			debug_print("[usb monitor] failed get device serial %s", libusb_strerror(ret.error));
			return 0;
		}

		if (usb_device_find(scope, ret.vid, ret.pid, serialString) != NULL)
		{
			debug_print("THAT IS IMPOSSIBLE!");
		}

		kburn_err_t r = open_single_usb_port(scope, dev);
		if (r != KBurnNoErr)
			debug_print("failed open single port: %ld", r);
	}
	else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event)
	{
		uint8_t serialString[MAX_SERIAL_LENGTH];
		struct usb_get_serial_ret ret = usb_get_serial(dev, serialString, MAX_SERIAL_LENGTH);
		if (ret.error != LIBUSB_SUCCESS)
		{
			debug_print("[usb monitor] failed get device serial %s", libusb_strerror(ret.error));
			return 0;
		}

		kburnDeviceNode *node = usb_device_find(scope, ret.vid, ret.pid, serialString);
		if (node != NULL)
			close_single_usb_port(scope, node);
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
	libusb_hotplug_deregister_callback(scope->usb->libusb, scope->usb->monitor_handle);
	scope->usb->monitor_enabled = false;
}

kburn_err_t usb_monitor_resume(KBCTX scope)
{
	debug_print("usb_monitor_resume()");
	if (scope->usb->monitor_enabled)
		return KBurnNoErr;

	//	if(!libUsbHasWathcer){
	// TODO: polling
	//	}

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
		scope,
		&scope->usb->monitor_handle);

	if (LIBUSB_SUCCESS != ret)
	{
		debug_print("error creating a hotplug callback: %s", libusb_strerror(ret));
	}

	return KBURN_ERROR_KIND_USB | ret;
}
