#include "base.h"
#include "context.h"
#include "debug/print.h"
#include "lifecycle.h"
#include "private-types.h"
#include <stdint.h>

kburn_err_t kburnOpenSerial(KBCTX scope, const char *path) {
	debug_print(KBURN_LOG_DEBUG, "kburnOpenSerial(%s)", path);
	return on_serial_device_attach(scope, path);
}

uint32_t baudrateHighValue = 460800;

void kburnSetHighSpeedValue(uint32_t baudrate) {
	debug_trace_function("%d", baudrate);
	baudrateHighValue = baudrate;
}
uint32_t kburnGetHighSpeedValue() { return baudrateHighValue; }

DEFINE_REGISTER_SWAPPER(kburnOnSerialConnect, scope->serial->on_verify, on_device_connect)
DEFINE_REGISTER_SWAPPER(kburnOnSerialConfirm, scope->serial->on_handle, on_device_handle)
