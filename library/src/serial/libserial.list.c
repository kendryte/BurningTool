#include "libserial.list.h"
#include "basic/errors.h"
#include "debug/print.h"
#include "lifecycle.h"
#include <sercomm/sercomm.h>

kburn_err_t init_list_all_serial_devices(KBCTX scope) {
	debug_print(KBURN_LOG_INFO, "[init] init_list()");
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
