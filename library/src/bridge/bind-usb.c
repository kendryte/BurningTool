#include "usb.h"
#include "protocol.h"

kburn_err_t usb_device_serial_bind(kburnDeviceNode *node)
{
	debug_print("usb_device_serial_bind:");
	m_assert(node->bind_id != 0, "call alloc_new_bind_id before this");

	char output[512];
	size_t buff_len = create_pair_protocol(node->bind_id, output, 512);

	for (int i = 5; i > 0; i--)
	{
		kburn_err_t r = usb_device_serial_print(node, (uint8_t *)output, buff_len);
		if (r != KBurnNoErr)
			return r;

		// TODO match
	}

	return KBurnNoErr;
}