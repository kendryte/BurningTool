#include "basic/array.h"
#include "basic/errors.h"
#include "basic/lock.h"
#include "basic/number.h"
#include "libusb.list.h"
#include "lifecycle.h"
#include "private-types.h"
#include "subsystem.h"
#include "components/call-user-handler.h"
#include "canaan-burn/canaan-burn.h"
#include <libusb.h>

kburn_err_t kburnOpenUsb(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *path) {
	if (!scope->usb->libusb) {
		usb_subsystem_init(scope);
	}

	libusb_device *dev = get_usb_device(scope->usb->libusb, vid, pid, path);

	if (dev == NULL) {
		return make_error_code(KBURN_ERROR_KIND_COMMON, KBurnUsbDeviceNotFound);
	}

	IfErrorReturn(open_single_usb_port(scope, dev, false, NULL));

	return KBurnNoErr;
}

kburn_err_t kburnPollUsb(KBCTX scope) {
	if (!scope->usb->subsystem_inited) {
		usb_subsystem_init(scope);
	}

	return init_list_all_usb_devices(scope);
}

DEFINE_REGISTER_SWAPPER(kburnOnUsbConnect, scope->usb->on_connect, on_device_connect)
DEFINE_REGISTER_SWAPPER(kburnOnUsbConfirm, scope->usb->on_handle, on_device_handle)

void kburnSetUsbFilter(KBCTX scope, int vid, int pid) {
	if (vid == KBURN_VIDPID_FILTER_DEFAULT) {
		scope->usb->filter.vid = DEFAULT_VID;
	} else {
		scope->usb->filter.vid = vid;
	}

	if (pid == KBURN_VIDPID_FILTER_DEFAULT) {
		scope->usb->filter.pid = DEFAULT_PID;
	} else {
		scope->usb->filter.pid = pid;
	}
}

kburnUsbDeviceList kburnGetUsbList(KBCTX scope) {
	if (scope->list2 == NULL) {
		scope->list2 = array_create(kburnUsbDeviceInfoSlice, 10);
		if (scope->list2 == NULL) {
			return (kburnUsbDeviceList){.size = 0, .list = NULL};
		}
		dispose_list_add(scope->disposables, toDisposable(array_destroy, scope->list2));
	}

	dynamic_array_t *array = scope->list2;
	ssize_t list_size = list_usb_ports(scope, array->body, array->size);
	if (list_size < 0) {
		return (kburnUsbDeviceList){.size = 0, .list = NULL};
	}
	if ((size_t)list_size > array->size) {
		array_fit(array, list_size + 5);
		list_size = list_usb_ports(scope, array->body, array->size);
	}

	array->length = MIN(list_size, array->size);

	return (kburnUsbDeviceList){.size = array->length, .list = array->body};
}

void kburnFreeUsbList(KBCTX scope) {
	array_destroy(scope->disposables, scope->list2);
}
