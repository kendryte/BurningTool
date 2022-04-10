#include "basic/errors.h"
#include "basic/sleep.h"
#include "baudtate.h"
#include "device.h"
#include "isp.h"
#include "low.h"
#include "private-types.h"
#include "components/device-link-list.h"
#include "canaan-burn/exported/serial.config.h"

slip_error_t _serial_isp_slip_send_request(kburnSerialDeviceNode *node, isp_request_t *command);

static bool greeting(kburnSerialDeviceNode *node, bool auto_switch) {
	debug_trace_function();

	make_serial_isp_packet(hello_pkt, 0);

	hello_pkt->op = ISP_NOP;

	if (!serial_isp_command_send_low_retry(node, hello_pkt, 3)) {
		debug_print(KBURN_LOG_ERROR, "greeting: FAILED");

		if (!auto_switch) {
			return false;
		}

		if (error_compare(node, KBURN_ERROR_KIND_SERIAL, SER_ETIMEDOUT)) {
			uint32_t want_speed;
			if (node->baudRate == KBURN_K510_BAUDRATE_DEFAULT) {
				debug_print(KBURN_LOG_ERROR, "  - error is timeout, retry high speed.");
				want_speed = baudrateHighValue;
			} else if (node->baudRate == baudrateHighValue) {
				debug_print(KBURN_LOG_ERROR, "  - error is timeout, retry low speed.");
				want_speed = KBURN_K510_BAUDRATE_DEFAULT;
			} else {
				return false;
			}

			if (!hackdev_serial_low_switch_baudrate(node, want_speed)) {
				debug_print(KBURN_LOG_ERROR, "  - can not switch baudrate");
				return false;
			}

			if (!serial_isp_command_send_low_retry(node, hello_pkt, 4)) {
				debug_print(KBURN_LOG_ERROR, "greeting: high/low speed also FAILED");
				return false;
			}
		} else {
			return false;
		}
	}

	debug_print(KBURN_LOG_DEBUG, "greeting: ok");
	return true;
}

bool kburnSerialIspGreeting(kburnSerialDeviceNode *node) {
	return greeting(node, true);
}

bool kburnSerialIspSetBaudrateHigh(kburnSerialDeviceNode *node) {
	return kburnSerialIspSetBaudrate(node, baudrateHighValue);
}

bool kburnSerialIspSetBaudrate(kburnSerialDeviceNode *node, uint32_t want_br) {
	debug_trace_function("baudrate=%d", want_br);
	if (want_br == node->baudRate) {
		debug_print(KBURN_LOG_DEBUG, "  - not change");
		return true;
	}

	make_serial_isp_packet(packet, sizeof(uint32_t));

	packet->op = ISP_UARTHS_BAUDRATE_SET;
	memcpy(packet->data, (void *)&want_br, sizeof(uint32_t));

	if (_serial_isp_slip_send_request(node, packet) != SLIP_NO_ERROR) {
		debug_print(KBURN_LOG_ERROR, "kburnSerialIspSetBaudrate: failed send");
		return false;
	}

	debug_print(KBURN_LOG_DEBUG, "kburnSerialIspSetBaudrate: do_sleep 1s");
	do_sleep(1000);

	if (hackdev_serial_low_switch_baudrate(node, want_br) && greeting(node, false)) {
		debug_print(KBURN_LOG_INFO, "switch driver attr success, baudrate to %d", want_br);
	} else {
		debug_print(KBURN_LOG_ERROR, "kburnSerialIspSetBaudrate: switch driver attr failed");
		if (!serial_low_switch_baudrate(node, want_br)) {
			debug_print(KBURN_LOG_ERROR, "kburnSerialIspSetBaudrate: failed re-open serial port");
			return false;
		}

		if (!greeting(node, false)) {
			debug_print(KBURN_LOG_ERROR, "kburnSerialIspSetBaudrate: re-greeting failed");
			return false;
		}
		debug_print(KBURN_LOG_INFO, "port re-opened with baudrate %d", want_br);
	}

	return true;
}

bool kburnSerialIspBootMemory(kburnSerialDeviceNode *node, kburn_mem_address_t address) {
	debug_trace_function("node[%s], 0x%X", node->deviceInfo.path, address);
	make_serial_isp_packet(packet, 0);

	packet->op = ISP_MEMORY_BOOT;
	packet->address = address;

	do_sleep(1000);
	for (int i = 0; i < 5; i++) {
		if (_serial_isp_slip_send_request(node, packet) != SLIP_NO_ERROR) {
			debug_print(KBURN_LOG_ERROR, "kburnSerialIspBootMemory: failed send");
			return false;
		}
		do_sleep(50);
	}

	debug_print(KBURN_LOG_INFO, "kburnSerialIspBootMemory: command sent (5 times), now switch to usb protocol!\n\n");

	if (hackdev_serial_low_switch_baudrate(node, KBURN_K510_BAUDRATE_USBISP)) {
		debug_print(KBURN_LOG_INFO, "switch driver attr success, baudrate to %d", KBURN_K510_BAUDRATE_USBISP);
	} else {
		debug_print(KBURN_LOG_ERROR, "switch driver attr failed");
		if (!serial_low_switch_baudrate(node, KBURN_K510_BAUDRATE_USBISP)) {
			debug_print(KBURN_LOG_ERROR, "failed re-open serial port");
			return false;
		}
		debug_print(KBURN_LOG_INFO, "port re-opened with baudrate %d", KBURN_K510_BAUDRATE_USBISP);
	}

	node->isSwitchIsp = true;
	serial_isp_delete(node);

	recreate_waitting_list(node->parent->_scope);

	return true;
}

bool kburnSerialIspMemoryWrite(
	kburnSerialDeviceNode *node, const kburn_mem_address_t address, const char *data, const size_t data_size, const on_write_progress cb, void *ctx) {
	make_serial_isp_packet(packet, BOARD_MEMORY_PAGE_SIZE);
	packet->op = ISP_MEMORY_WRITE;

	const uint32_t pages = (data_size + BOARD_MEMORY_PAGE_SIZE - 1) / BOARD_MEMORY_PAGE_SIZE;
	debug_trace_function("base=0x%X, size=" FMT_SIZET ", pages=%d", address, data_size, pages);

	if (cb) {
		cb(ctx, get_node(node), 0, data_size);
	}

	for (uint32_t page = 0; page < pages; page++) {
		uint32_t offset = page * BOARD_MEMORY_PAGE_SIZE;
		packet->address = address + offset;
		if (offset + BOARD_MEMORY_PAGE_SIZE > data_size) {
			packet->data_len = data_size % BOARD_MEMORY_PAGE_SIZE;
		} else {
			packet->data_len = BOARD_MEMORY_PAGE_SIZE;
		}

		debug_print(KBURN_LOG_TRACE, "[mem write]: page=%u [0x%X], size=%u", page, packet->address, packet->data_len);

		memcpy(packet->data, data + offset, packet->data_len);

		packet->checksum = 0;
		if (!serial_isp_command_send_low_retry(node, packet, 3)) {
			return false;
		}

		if (cb) {
			cb(ctx, get_node(node), page * BOARD_MEMORY_PAGE_SIZE + packet->data_len, data_size);
		}
	}

	return true;
}

bool kburnSerialIspRunProgram(
	kburnSerialDeviceNode *node, const void *programBuffer, size_t programBufferSize, on_write_progress page_callback, void *ctx) {
	debug_trace_function("node[%s]", node->deviceInfo.path);

	if (!greeting(node, false)) {
		return false;
	}

	if (!kburnSerialIspMemoryWrite(node, KBURN_PROGRAM_BASE_ADDR, programBuffer, programBufferSize, page_callback, ctx)) {
		return false;
	}

	do_sleep(100);
	if (!greeting(node, false)) {
		debug_print(KBURN_LOG_ERROR, "	re-greeting failed...");
		return false;
	}

	if (!kburnSerialIspBootMemory(node, KBURN_PROGRAM_BASE_ADDR)) {
		return false;
	}
	return true;
}

#include "generated.usb_isp_buffer.h"
bool kburnSerialIspSwitchUsbMode(kburnSerialDeviceNode *node, on_write_progress page_callback, void *ctx) {
	return kburnSerialIspRunProgram(node, usb_isp_buffer, usb_isp_buffer_size, page_callback, ctx);
}

size_t kburnGetUsbIspProgramSize() {
	return usb_isp_buffer_size;
}
