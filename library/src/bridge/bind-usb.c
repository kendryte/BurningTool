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

	for (int i = 5; i > 0; i--) {
		kburn_err_t r = usb_device_serial_print(node, (uint8_t *)output, buff_len);
		if (r != KBurnNoErr) {
			set_kb_error(node, r, "failed send serial print command");
			return r;
		}

		do_sleep(500);

		if (node->serial->isUsbBound) {
			debug_print(KBURN_LOG_INFO, " * bind success @ usb");
			return KBurnNoErr;
		}
	}

	debug_print(KBURN_LOG_INFO, " * bind " COLOR_FMT("failed") " @ usb", COLOR_ARG(YELLOW));
	return KBurnNoErr;
}
