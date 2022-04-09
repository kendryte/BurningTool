#include "libserial.list.h"
#include "basic/errors.h"
#include "driver.h"
#include "lifecycle.h"
#include <sercomm/sercomm.h>
#include "debug/print.h"

kburn_err_t init_list_all_serial_devices(KBCTX scope) {
	debug_trace_function();
	ser_dev_list_t *lst;

	lst = ser_dev_list_get();
	if (!lst) {
		debug_print(KBURN_LOG_ERROR, "serial port list get failed: %s", sererr_last());
		return make_error_code(KBURN_ERROR_KIND_COMMON, KBurnNoMemory);
	}

	ser_dev_list_t *item;

	ser_dev_list_foreach(item, lst) {
		if (prefix("/dev/ttyS", item->dev.path)) {
			continue;
		}
		debug_print(KBURN_LOG_INFO, "[init]   * %s", item->dev.path);
		on_serial_device_attach(scope, item->dev.path, true);
	}

	ser_dev_list_destroy(lst);
	return KBurnNoErr;
}

ssize_t list_serial_ports(KBCTX UNUSED(scope), struct kburnSerialDeviceInfoSlice list[], size_t max_size) {
	debug_trace_function();

	ser_dev_list_t *lst = ser_dev_list_get();
	if (!lst) {
		debug_print(KBURN_LOG_ERROR, "serial port list get failed: %s", sererr_last());
		return -1;
	}

	size_t count = 0;

	ser_dev_list_t *item;
	ser_dev_list_foreach(item, lst) { count++; }

	size_t i = 0;
	ser_dev_list_foreach(item, lst) {
		if (prefix("/dev/ttyS", item->dev.path)) {
			continue;
		}

		if (i >= max_size) {
			i++;
			continue;
		}

		kburnSerialDeviceInfo e = driver_get_devinfo(item->dev.path);

		list[i].isUSB = e.isUSB;
		list[i].isTTY = e.isTTY;
		strcpy(list[i].path, e.path);

		list[i].usbIdVendor = e.usbIdVendor;
		list[i].usbIdProduct = e.usbIdProduct;
		list[i].usbIdRevision = e.usbIdRevision;

#ifdef __linux__
		strcpy(list[i].usbDriver, e.usbDriver);
		list[i].deviceMajor = e.deviceMajor;
		list[i].deviceMinor = e.deviceMinor;
#elif WIN32
		strcpy(list[i].title, e.title);
		strcpy(list[i].hwid, e.hwid);
#endif

		i++;
	}

	ser_dev_list_destroy(lst);
	return i;
}
