#include "base.h"
#include "context.h"
#include "basic/array.h"
#include "basic/number.h"
#include "libserial.list.h"
#include "lifecycle.h"
#include "private-types.h"
#include "subsystem.h"
#include <stdint.h>
#include "debug/print.h"

kburn_err_t kburnOpenSerial(KBCTX scope, const char *path) {
	debug_print(KBURN_LOG_DEBUG, "kburnOpenSerial(%s)", path);
	return on_serial_device_attach(scope, path, false);
}

uint32_t baudrateHighValue = 460800;

void kburnSetHighSpeedValue(uint32_t baudrate) {
	debug_trace_function("%d", baudrate);
	baudrateHighValue = baudrate;
}
uint32_t kburnGetHighSpeedValue() { return baudrateHighValue; }

DEFINE_REGISTER_SWAPPER(kburnOnSerialConnect, scope->serial->on_verify, on_device_connect)
DEFINE_REGISTER_SWAPPER(kburnOnSerialConfirm, scope->serial->on_handle, on_device_handle)

PUBLIC kburn_err_t kburnPollSerial(KBCTX scope) {
	if (!scope->serial->subsystem_inited)
		serial_subsystem_init(scope);

	return init_list_all_serial_devices(scope);
}

kburnSerialDeviceList kburnGetSerialList(KBCTX scope) {
	if (scope->list1 == NULL) {
		scope->list1 = array_create(struct kburnSerialDeviceInfoSlice, 10);
		if (scope->list1 == NULL) {
			return (kburnSerialDeviceList){.size = 0, .list = NULL};
		}
		dispose_list_add(scope->disposables, toDisposable(array_destroy, scope->list1));
	}

	dynamic_array_t *array = scope->list1;
	ssize_t list_size = list_serial_ports(scope, array->body, array->size);
	if (list_size < 0) {
		return (kburnSerialDeviceList){.size = 0, .list = NULL};
	}
	if ((size_t)list_size > array->size) {
		array_fit(array, list_size + 5);
		list_size = list_serial_ports(scope, array->body, array->size);
	}

	array->length = MIN(list_size, array->size);

	return (kburnSerialDeviceList){.size = array->length, .list = array->body};
}

void kburnFreeSerialList(KBCTX scope) { array_destroy(scope->disposables, scope->list1); }
