#pragma once

#include <stdlib.h>
#include "global.h"
#include "thread.h"
#include "usb.types.h"

typedef struct usb_subsystem_context
{
	struct
	{
		int vid;
		int pid;
	} filter;

	bool subsystem_inited;
	bool detach_kernel_driver;

	on_device_handle handle_callback;
	void *handle_callback_ctx;

	struct libusb_context *libusb;
	bool monitor_prepared;
	bool monitor_enabled;
	libusb_hotplug_callback_handle monitor_handle;

	kbthread libusb_thread;

	struct queue_info *queue;
	kbthread event_thread;
} usb_subsystem_context;

kburn_err_t usb_subsystem_init(KBCTX scope);
void usb_subsystem_deinit(KBCTX scope);

kburn_err_t usb_monitor_prepare(KBCTX scope);
void usb_monitor_destroy(KBCTX scope);
void usb_monitor_pause(KBCTX scope);
kburn_err_t usb_monitor_resume(KBCTX scope);

int usb_get_vid_pid_path(struct libusb_device *dev, uint16_t *out_vid, uint16_t *out_pid, uint8_t out_path[MAX_PATH_LENGTH]);
int usb_get_device_path(struct libusb_device *dev, uint8_t path[MAX_PATH_LENGTH]);
int usb_get_device_serial(libusb_device *dev, libusb_device_handle *handle, uint8_t *output);

libusb_device *get_usb_device(struct libusb_context *libusb, uint16_t vid, uint16_t pid, const uint8_t *path);
void free_got_usb_device(libusb_device *dev);
void free_all_unopend_usb_info(kburnUsbDeviceInfo **list);
int get_all_unopend_usb_info(KBCTX scope, int vid, int pid, kburnUsbDeviceInfo **ret);
kburn_err_t init_list_all_usb_devices(KBCTX scope);

DECALRE_DISPOSE_HEADER(destroy_usb_port, kburnUsbDeviceNode);
kburn_err_t open_single_usb_port(KBCTX scope, struct libusb_device *dev, kburnDeviceNode **out_node);
kburnDeviceNode *usb_device_find(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *serial);

#define debug_print_libusb_result(msg, err, ...) \
	debug_print(err < 0 ? KBURN_LOG_ERROR : KBURN_LOG_DEBUG, msg ": %s[%d] %s", __VA_ARGS__ __VA_OPT__(, ) libusb_error_name(err), (int)err, libusb_strerror(err));

#define debug_print_libusb_error(msg, err, ...) __extension__({        \
	if (err < 0)                                                       \
	{                                                                  \
		debug_print_libusb_result(msg, err __VA_OPT__(, ) __VA_ARGS__) \
	}                                                                  \
})

#define CheckLibusbError(action) __extension__({           \
	int _err = action;                                     \
	if (_err < LIBUSB_SUCCESS)                             \
	{                                                      \
		debug_print_libusb_error(#action "-failed", _err); \
		return _err;                                       \
	}                                                      \
	_err;                                                  \
})

#define check_libusb(err) (err >= LIBUSB_SUCCESS)
#define set_node_error_with_log(err, msg, ...)                      \
	debug_print_libusb_result(msg, err __VA_OPT__(, ) __VA_ARGS__); \
	copy_last_libusb_error(node, err);
#define IfUsbErrorLogReturn(action) IfErrorReturn(check_libusb, action, set_node_error_with_log)
#define set_node_error(err, msg) copy_last_libusb_error(node, err);
#define IfUsbErrorReturn(action) IfErrorReturn(check_libusb, action, set_node_error)

kburn_err_t usb_device_hello(kburnDeviceNode *node);
kburn_err_t usb_device_serial_print(kburnDeviceNode *node, const uint8_t *buff, size_t buff_size);
kburn_err_t usb_device_serial_bind(kburnDeviceNode *node);

const char *usb_debug_path_string(const uint8_t *path);
