#pragma once

#include "context.h"
#include <libusb.h>

struct passing_data {
	KBCTX scope;
	struct kburnUsbDeviceInfoSlice dev;
	libusb_hotplug_event event;
};

kburn_err_t usb_monitor_prepare(KBCTX scope);
void usb_monitor_destroy(KBCTX scope);
kburn_err_t usb_monitor_resume(KBCTX scope);
void usb_monitor_pause(KBCTX scope);

void push_libusb_event(KBCTX scope, libusb_hotplug_event event, const struct kburnUsbDeviceInfoSlice *devInfo);

kburn_err_t usb_monitor_polling_prepare(KBCTX scope);
void usb_monitor_polling_destroy(KBCTX scope);
kburn_err_t usb_monitor_polling_resume(KBCTX scope);
void usb_monitor_polling_pause(KBCTX scope);

kburn_err_t usb_monitor_callback_prepare(KBCTX scope);
void usb_monitor_callback_destroy(KBCTX scope);
kburn_err_t usb_monitor_callback_resume(KBCTX scope);
void usb_monitor_callback_pause(KBCTX scope);
