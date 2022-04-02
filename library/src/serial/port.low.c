#include "serial.h"

static bool open(kburnSerialDeviceNode *node, ser_opts_t opts)
{
	m_assert(node->m_dev_handle == NULL, "duplicate call to serial port open");

	ser_t *handle = ser_create();

	debug_print(KBURN_LOG_INFO, "low open: %s br=%d, bs=%d, par=%d, sb=%d", opts.port, opts.baudrate, opts.bytesz, opts.parity, opts.stopbits);
	int32_t r = ser_open(handle, &opts);

	if (r < 0)
	{
		copy_last_serial_io_error(node, r);
		ser_destroy(handle);
		node->isOpen = false;
		return false;
	}

	node->baudRate = opts.baudrate;

	debug_print(KBURN_LOG_DEBUG, "  - open success");
	node->m_dev_handle = handle;
	node->isOpen = true;

	return true;
}

void serial_low_close(kburnSerialDeviceNode *node)
{
	debug_trace_function("node[%s]", node->path);
	if (node->isOpen)
	{
		ser_close(node->m_dev_handle);
		node->isOpen = false;
	}
	if (node->m_dev_handle)
	{
		ser_destroy(node->m_dev_handle);
		node->m_dev_handle = NULL;
	}
}

bool serial_low_open(kburnSerialDeviceNode *node)
{
	debug_trace_function("node[%s]", node->path);
	return open(node, get_current_serial_options(node->path));
}

void serial_low_flush_all(kburnSerialDeviceNode *node)
{
	ser_flush(node->m_dev_handle, SER_QUEUE_ALL);
}

bool serial_low_switch_baudrate(kburnSerialDeviceNode *node, uint32_t speed)
{
	serial_low_close(node);
	ser_opts_t opts = get_current_serial_options(node->path);
	opts.baudrate = speed;
	return open(node, opts);
}

void serial_low_output_nulls(kburnSerialDeviceNode *node, bool push_end_char)
{
	debug_trace_function("push_end_char=%d", push_end_char);
	static char buff[128] = {
		0,
	};
	if (push_end_char)
	{
		ser_write(node->m_dev_handle, buff, 64, NULL);
		char ending = '\xC0';
		ser_write(node->m_dev_handle, &ending, 1, NULL);
		ser_write(node->m_dev_handle, buff, 64, NULL);
	}
	else
	{
		ser_write(node->m_dev_handle, buff, 128, NULL);
	}
	serial_low_flush_all(node);
	serial_low_drain_input(node);
}
int32_t serial_low_drain_input(kburnSerialDeviceNode *node)
{
	int32_t bytes = 0;
	size_t ava = 0, recv = 0;
	while (true)
	{
		int32_t err = ser_available(node->m_dev_handle, &ava);
		if (err != 0)
		{
			copy_last_serial_io_error(node, err);
			return -1;
		}

		if (ava > 0)
		{
			uint8_t buff[32];
			ser_read(node->m_dev_handle, buff, 32, &recv);
			bytes += recv;
			print_buffer(KBURN_LOG_WARN, "extra bytes:", buff, recv);
		}
		else
		{
			return bytes;
		}
	}
}
