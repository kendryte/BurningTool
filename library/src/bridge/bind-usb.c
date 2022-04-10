#include "../usb/isp.h"
#include "basic/sleep.h"
#include "device.h"
#include "protocol.h"
#include "debug/print.h"

kburn_err_t usb_device_serial_bind(kburnDeviceNode *node) {
	debug_trace_function();
	m_assert(node->bind_id != 0, "call alloc_new_bind_id before this");

	char output[512];
	size_t buff_len = create_pair_protocol(node->bind_id, output, 512);

	for (int i = 10; i > 0; i--) {
		if (!usb_device_serial_print(node, (uint8_t *)output, buff_len)) {
			return node->error->code;
		}

		do_sleep(1000);
		debug_print(KBURN_LOG_TRACE, "wait usb: %d", i);

		if (node->serial->isUsbBound) {
			debug_print(KBURN_LOG_INFO, " * bind success @ usb");
			clear_error(node);
			return KBurnNoErr;
		}
	}

	debug_print(KBURN_LOG_INFO, " * bind " COLOR_FMT("failed") " @ usb", COLOR_ARG(YELLOW));
	return KBurnNoErr;
}
