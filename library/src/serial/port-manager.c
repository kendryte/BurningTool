#include "serial.h"

void on_device_attach(const char *path)
{
	debug_print("on_device_attach(%s)", path);

	if (prefix("/dev/tty", path))
	{
		const char n = path[8];
		if (n == '\0' || n == 'S' || (n >= '0' && n <= 9))
		{
			debug_print("  - ignore tty");
			return;
		}
	}

	kburnSerialNode *node = create_port(path);

	if (mon.verify_callback != NULL)
	{
		if (!mon.verify_callback(node))
		{
			debug_print("verify return false");
			destroy_port(node);
			return;
		}
	}

	if (!node->isOpen)
	{
		if (!serial_low_open(node))
		{
			debug_print("open failed");
			destroy_port(node);
			return;
		}
	}

	if (!node->isConfirm)
	{
		if (!confirm_port_is_ready(node))
		{
			debug_print("confirm failed");
			destroy_port(node);
			return;
		}
	}

	mon.handler_callback(node);
}
