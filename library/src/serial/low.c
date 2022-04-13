#include "low.h"
#include "device.h"
#include "settings.h"

static bool open(kburnSerialDeviceNode *node, ser_opts_t opts) {
	m_assert(node->m_dev_handle == NULL, "duplicate call to serial port open");

	ser_t *handle = ser_create();

	debug_print(KBURN_LOG_INFO, "low open: %s br=%d, bs=%d, par=%d, sb=%d", opts.port, opts.baudrate, opts.bytesz, opts.parity, opts.stopbits);
	int32_t r = ser_open(handle, &opts);

	if (r < 0) {
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

void serial_low_close(kburnSerialDeviceNode *node) {
	debug_trace_function("node[%s]", node->deviceInfo.path);
	if (node->isOpen) {
		ser_close(node->m_dev_handle);
		node->isOpen = false;
	}
	if (node->m_dev_handle) {
		ser_destroy(node->m_dev_handle);
		node->m_dev_handle = NULL;
	}
}

bool serial_low_open(kburnSerialDeviceNode *node) {
	debug_trace_function("node[%s]", node->deviceInfo.path);
	return open(node, get_current_serial_options(scopeOf(node), node->deviceInfo.path));
}

void serial_low_flush_all(kburnSerialDeviceNode *node) {
	ser_flush(node->m_dev_handle, SER_QUEUE_ALL);
}

bool serial_low_switch_baudrate(kburnSerialDeviceNode *node, uint32_t speed) {
	serial_low_close(node);
	ser_opts_t opts = get_current_serial_options(scopeOf(node), node->deviceInfo.path);
	opts.baudrate = speed;
	return open(node, opts);
}

bool serial_low_write_data(kburnSerialDeviceNode *node, const void *data, size_t data_length, size_t *written) {
	print_buffer(KBURN_LOG_BUFFER, "PC→MCU", data, data_length);
	int r = ser_write(node->m_dev_handle, data, data_length, written);
	if (r != 0) {
		copy_last_serial_io_error(node, r);
		return false;
	}

	return true;
}

bool serial_low_read_data(kburnSerialDeviceNode *node, uint8_t **buffer, size_t *buffer_size, size_t *read) {
	*read = 0;
	size_t ava = 0;
	int32_t err = ser_available(node->m_dev_handle, &ava);
	if (err != 0) {
		copy_last_serial_io_error(node, err);
		return false;
	}

	if (ava > *buffer_size) {
		debug_print(KBURN_LOG_WARN, "realloc happed, increase buffer size: " FMT_SIZET, ava);
		*buffer_size = ava + 32;
		*buffer = realloc(*buffer, *buffer_size);
	}
	if (ava == 0) {
		return true;
	}

	err = ser_read(node->m_dev_handle, *buffer, *buffer_size, read);
	if (err != 0) {
		copy_last_serial_io_error(node, err);
		return false;
	}

	print_buffer(KBURN_LOG_BUFFER, "MCU→PC", *buffer, *buffer_size);
	return true;
}

void serial_low_output_nulls(kburnSerialDeviceNode *node, bool push_end_char) {
	debug_trace_function("push_end_char=%d", push_end_char);
	static char buff[128] = {
		0,
	};
	if (push_end_char) {
		ser_write(node->m_dev_handle, buff, 64, NULL);
		char ending = '\xC0';
		ser_write(node->m_dev_handle, &ending, 1, NULL);
		ser_write(node->m_dev_handle, buff, 64, NULL);
	} else {
		ser_write(node->m_dev_handle, buff, 128, NULL);
	}
	serial_low_flush_all(node);
	serial_low_drain_input(node, NULL);
}
bool serial_low_drain_input(kburnSerialDeviceNode *node, size_t *drop_bytes) {
	size_t ava = 0, recv = 0;
	uint8_t buff[PRINT_BUFF_MAX];
	while (true) {
		int32_t err = ser_available(node->m_dev_handle, &ava);
		if (err != 0) {
			copy_last_serial_io_error(node, err);
			return false;
		}

		if (ava > 0) {
			ser_read(node->m_dev_handle, buff, PRINT_BUFF_MAX, &recv);
			if (drop_bytes) {
				*drop_bytes += recv;
			}
			print_buffer(KBURN_LOG_WARN, "drop:", buff, recv);
		} else {
			return true;
		}
	}
}
