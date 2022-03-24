#include "serial.h"

slip_error_t _serial_isp_slip_send_request(kburnSerialDeviceNode *node, isp_request_t *command);

static bool greeting(kburnSerialDeviceNode *node, bool auto_switch)
{
	debug_print("greeting:");
	make_serial_isp_packet(hello_pkt, 0);

	hello_pkt->op = ISP_NOP;

	if (!serial_isp_command_send_low_retry(node, hello_pkt, 3))
	{
		debug_print("greeting: FAILED");

		if (!auto_switch)
			return false;

		if (error_compare(node, KBURN_ERROR_KIND_SERIAL, SER_ETIMEDOUT))
		{
			uint32_t want_speed;
			if (node->baudRate == KBURN_K510_BAUDRATE_DEFAULT)
			{
				debug_print("  - error is timeout, retry high speed.");
				want_speed = baudrateHighValue;
			}
			else if (node->baudRate == baudrateHighValue)
			{
				debug_print("  - error is timeout, retry low speed.");
				want_speed = KBURN_K510_BAUDRATE_DEFAULT;
			}
			else
				return false;

			if (!hackdev_serial_low_switch_baudrate(node, want_speed))
			{
				debug_print("  - can not switch baudrate");
				return false;
			}

			if (!serial_isp_command_send_low_retry(node, hello_pkt, 4))
			{
				debug_print("greeting: high/low speed also FAILED");
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	debug_print("greeting: ok");
	return true;
}

bool kburnSerialIspGreeting(kburnSerialDeviceNode *node)
{
	return greeting(node, true);
}

bool kburnSerialIspSetBaudrateHigh(kburnSerialDeviceNode *node)
{
	return kburnSerialIspSetBaudrate(node, baudrateHighValue);
}

bool kburnSerialIspSetBaudrate(kburnSerialDeviceNode *node, uint32_t want_br)
{
	debug_print("kburnSerialIspSetBaudrate: %d", want_br);
	if (want_br == node->baudRate)
	{
		debug_print("  - not change");
		return true;
	}

	make_serial_isp_packet(packet, sizeof(uint32_t));

	packet->op = ISP_UARTHS_BAUDRATE_SET;
	memcpy(packet->data, (void *)&want_br, sizeof(uint32_t));

	if (_serial_isp_slip_send_request(node, packet) != SLIP_NO_ERROR)
	{
		debug_print("kburnSerialIspSetBaudrate: failed send");
		return false;
	}

	debug_print("kburnSerialIspSetBaudrate: do_sleep 1s");
	do_sleep(1000);

	if (hackdev_serial_low_switch_baudrate(node, want_br) && greeting(node, false))
	{
		debug_print("kburnSerialIspSetBaudrate: switch driver attr success");
	}
	else
	{
		debug_print("kburnSerialIspSetBaudrate: switch driver attr failed");
		if (!serial_low_switch_baudrate(node, want_br))
		{
			debug_print("kburnSerialIspSetBaudrate: failed switch driver attr");
			return false;
		}

		if (!greeting(node, false))
		{
			debug_print("kburnSerialIspSetBaudrate: re-greeting failed");
			return false;
		}
	}

	debug_print("kburnSerialIspSetBaudrate: ok");

	return true;
}

bool kburnSerialIspBootMemory(kburnSerialDeviceNode *node, kburn_address_t address)
{
	debug_print("kburnSerialIspBootMemory(node[%s], 0x%X)", node->path, address);
	make_serial_isp_packet(packet, 0);

	packet->op = ISP_MEMORY_BOOT;
	packet->address = address;

	if (_serial_isp_slip_send_request(node, packet) != SLIP_NO_ERROR)
	{
		debug_print("kburnSerialIspBootMemory: failed send");
		return false;
	}

	debug_print("kburnSerialIspBootMemory: command sent, now switch to usb protocol!\n\n");
	return true;
}

__attribute__((access(read_only, 3, 4))) bool
kburnSerialIspMemoryWrite(kburnSerialDeviceNode *node, const kburn_address_t address, const char *data, const size_t data_size, const on_write_progress cb, void *ctx)
{
	make_serial_isp_packet(packet, BOARD_MEMORY_PAGE_SIZE);
	packet->op = ISP_MEMORY_WRITE;

	const uint32_t pages = (data_size + BOARD_MEMORY_PAGE_SIZE - 1) / BOARD_MEMORY_PAGE_SIZE;
	debug_print("kburnSerialIspMemoryWrite: base=0x%X, size=%ld, pages=%d", address, data_size, pages);

	if (cb)
		cb(get_node(node), 0, data_size, ctx);

	for (uint32_t page = 0; page < pages; page++)
	{
		uint32_t offset = page * BOARD_MEMORY_PAGE_SIZE;
		packet->address = address + offset;
		if (offset + BOARD_MEMORY_PAGE_SIZE > data_size)
			packet->data_len = data_size % BOARD_MEMORY_PAGE_SIZE;
		else
			packet->data_len = BOARD_MEMORY_PAGE_SIZE;

		debug_print("[mem write]: page=%u [0x%X], size=%u", page, packet->address, packet->data_len);

		memcpy(packet->data, data + offset, packet->data_len);

		packet->checksum = 0;
		if (!serial_isp_command_send_low_retry(node, packet, 3))
		{
			return false;
		}

		if (cb)
			cb(get_node(node), page * BOARD_MEMORY_PAGE_SIZE + packet->data_len, data_size, ctx);
	}

	debug_print("kburnSerialIspMemoryWrite: DONE");
	return true;
}

bool kburnSerialIspRunProgram(kburnSerialDeviceNode *node, const void *programBuffer, size_t programBufferSize, on_write_progress page_callback, void *ctx)
{
	debug_print("kburnSerialIspRunProgram(node[%s])", node->path);

	greeting(node, false);

	if (!kburnSerialIspMemoryWrite(node, KBURN_PROGRAM_BASE_ADDR, programBuffer, programBufferSize, page_callback, ctx))
	{
		return false;
	}

	do_sleep(100);
	if (!greeting(node, false))
	{
		debug_print("	re-greeting failed...");
		return false;
	}

	if (!kburnSerialIspBootMemory(node, KBURN_PROGRAM_BASE_ADDR))
	{
		return false;
	}
	return true;
}

#include "usb_isp_buffer.h"
bool kburnSerialIspSwitchUsbMode(kburnSerialDeviceNode *node, on_write_progress page_callback, void *ctx)
{
	return kburnSerialIspRunProgram(node, usb_isp_buffer, usb_isp_buffer_size, page_callback, ctx);
}
