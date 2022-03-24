#pragma once

#include <stdlib.h>
#include "global.h"
#include "./usb.types.h"

typedef struct usb_subsystem_context
{
	struct
	{
		int vid;
		int pid;
	} filter;

	struct libusb_context *libusb;
	bool monitor_prepared;
	bool monitor_enabled;
	libusb_hotplug_callback_handle monitor_handle;

	kbthread libusb_thread;

	queue_t queue;
	kbthread event_thread;
} usb_subsystem_context;

kburn_err_t usb_subsystem_init(KBCTX scope);
void usb_subsystem_deinit(KBCTX scope);
kburn_err_t usb_monitor_prepare(KBCTX scope);
void usb_monitor_destroy(KBCTX scope);
void usb_monitor_pause(KBCTX scope);
kburn_err_t usb_monitor_resume(KBCTX scope);

int usb_get_vid_pid_path(struct libusb_device *dev, uint16_t *out_vid, uint16_t *out_pid, uint8_t *out_path);
int usb_get_device_path(struct libusb_device *dev, uint8_t *path);
int usb_get_device_serial(libusb_device *dev, libusb_device_handle *handle, uint8_t *output);

libusb_device *get_usb_device(struct libusb_context *libusb, uint16_t vid, uint16_t pid, const uint8_t *serial);
void free_got_usb_device(libusb_device *dev);
void free_all_unopend_usb_info(kburnUsbDeviceInfo **list);
int get_all_unopend_usb_info(KBCTX scope, int vid, int pid, kburnUsbDeviceInfo **ret);
kburn_err_t init_list_all_usb_devices(KBCTX scope);

kburn_err_t open_single_usb_port(KBCTX scope, struct libusb_device *dev);
kburn_err_t close_single_usb_port(KBCTX scope, kburnDeviceNode *dev);
kburnDeviceNode *usb_device_find(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *serial);

void destroy_usb_port(KBCTX scope, kburnDeviceNode *device);
kburn_err_t create_usb_port(KBCTX scope, kburnDeviceNode *device);

kburn_err_t usb_lowlevel_command_send(libusb_device_handle *handle, uint8_t endpoint,
									  const usbIspCommandPacket cdb, uint8_t direction, int data_length, uint32_t operation_index);
kburn_err_t usb_lowlevel_status_read(libusb_device_handle *handle, uint8_t endpoint, uint32_t expected_operation_index);
kburn_err_t usb_lowlevel_error_read(libusb_device_handle *handle, uint8_t endpoint_in, uint8_t endpoint_out);

#define debug_print_libusb_error(msg, err) \
	if (err < 0)                           \
		debug_print("%s: %s[%d] %s", msg, libusb_error_name(err), (int)err, libusb_strerror(err));

// void debug_print_libusb_error(const char *msg, int libusb_err);

kburn_err_t usb_device_hello(kburnDeviceNode *node);
kburn_err_t usb_device_serial_bind(kburnDeviceNode *node);

#ifndef NDEBUG
const char *_debug_path_string(const uint8_t *path);
#define debug_path_string(x) _debug_path_string(x)
#else
#define debug_path_string(x) ""
#endif
