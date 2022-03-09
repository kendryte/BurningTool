#include "serial.h"

bool serial_low_open(kburnSerialNode *node)
{
	debug_print("serial_low_open(node[%s])", node->path);
	ser_t *handle = ser_create();
	int32_t r = serial_open_with_current_config(handle, node->path);

	if (r < 0)
	{
		node->isError = KBurnSerialFailedOpen;
		copy_last_error(node);

		ser_destroy(handle);

		return false;
	}

	debug_print("  - open success");
	node->m_dev_handle = handle;
	node->isOpen = true;

	return handle;
}
