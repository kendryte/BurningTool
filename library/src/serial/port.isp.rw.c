#include "slip.h"
#include "serial.h"

void recv_message(void *_ctx, uint8_t *UNUSED(data), uint32_t size)
{
	kburnSerialDeviceNode *node = _ctx;
	node->isp->has_response = true;
	node->isp->main_buffer_length = size;

	return;
}

uint8_t write_byte(void *_ctx, uint8_t byte, bool end)
{
	kburnSerialDeviceNode *node = _ctx;

	node->isp->send_buffer[node->isp->send_buffer_length] = byte;
	node->isp->send_buffer_length++;

	if (end || node->isp->send_buffer_length >= SERIAL_CHUNK_SIZE)
	{
		size_t sent = 0;
		ser_write(node->m_dev_handle, node->isp->send_buffer, node->isp->send_buffer_length, &sent);

		print_buffer("PC→MCU", node->isp->send_buffer, node->isp->send_buffer_length);

		node->isp->send_buffer_length = 0;
	}
	if (end)
		serial_low_flush_all(node);

	return 1;
}

void serial_isp_open(kburnSerialDeviceNode *node)
{
	debug_print("serial_isp_open(node[%s])", node->path);
	isp_state *isp = malloc(sizeof(isp_state));

	isp->descriptor.buf = isp->main_buffer;
	isp->descriptor.buf_size = MAX_COMMAND_SIZE;
	isp->descriptor.crc_seed = 0xFFFF;
	isp->descriptor.recv_message = recv_message;
	isp->descriptor.write_byte = write_byte;
	isp->descriptor.context = node;
	isp->descriptor.crc_disabled = true;

	isp->main_buffer_length = 0;
	isp->send_buffer_length = 0;

	slip_init(&isp->slip, &isp->descriptor);

	node->isp = isp;
}

void serial_isp_delete(kburnSerialDeviceNode *node)
{
	debug_print("\tserial_isp_delete()");
	if (node->isp)
	{
		free(node->isp);
		node->isp = NULL;
	}
}

slip_error_t _serial_isp_slip_send_request(kburnSerialDeviceNode *node, isp_request_t *command)
{
	serial_low_drain_input(node);
	serial_isp_calculate_checksum(command);

	slip_error_t ret = slip_send_message(&node->isp->slip, (void *)command, serial_isp_packet_size(command));

	if (ret != SLIP_NO_ERROR)
		slip_error(node, ret);

	return ret;
}

isp_response_t *serial_isp_command_send_low_retry(kburnSerialDeviceNode *node, isp_request_t *command, int tries)
{
	for (int curr = 1; curr < tries; curr++)
	{
		isp_response_t *r = serial_isp_command_send_low(node, command);
		if (r)
			return r;

		debug_print("failed retry: %d/%d", curr + 1, tries);
		bool isWiredState = error_compare(node, KBURN_ERROR_KIND_COMMON, KBurnProtocolOpMismatch);
		clear_error(node);
		serial_low_output_nulls(node, isWiredState && (curr % 2 == 1));
	}
	return serial_isp_command_send_low(node, command);
}

isp_response_t *serial_isp_command_send_low(kburnSerialDeviceNode *node, isp_request_t *command)
{
	isp_response_t *ret = NULL;

	assert((!node->serial_isp_busy) && "serial isp is running.");
	node->serial_isp_busy = true;
	node->isp->has_response = false;
	node->isp->main_buffer_length = 0;

	if (_serial_isp_slip_send_request(node, command) != SLIP_NO_ERROR)
	{
		goto exit;
	}

#ifndef NDEBUG
	uint8_t debug_buffer[SERIAL_CHUNK_SIZE];
	memset(debug_buffer, 0, SERIAL_CHUNK_SIZE);
	size_t debug_buffer_length = 0;
#endif

	uint8_t byte = '\0';
	while (!node->isp->has_response)
	{
		size_t recv;
		int32_t err = ser_read_wait(node->m_dev_handle);
		if (err != 0)
		{
			copy_last_serial_io_error(node, err);
			goto exit;
		}

		err = ser_read(node->m_dev_handle, &byte, 1, &recv);
		if (err != 0)
		{
			copy_last_serial_io_error(node, err);
			goto exit;
		}

#ifndef NDEBUG
		if (debug_buffer_length < SERIAL_CHUNK_SIZE)
		{
			debug_buffer[debug_buffer_length] = byte;
			debug_buffer_length++;
		}
#endif

		assert(recv == 1);
		slip_error_t err2 = slip_read_byte(&node->isp->slip, byte);
		if (err2 != SLIP_NO_ERROR)
		{
			slip_error(node, err2);
			goto exit;
		}
	}

	print_buffer("MCU→PC", debug_buffer, debug_buffer_length);
	print_buffer("[response]", node->isp->main_buffer, node->isp->main_buffer_length);

	isp_response_t *response = (void *)node->isp->main_buffer;
	if (response->op != command->op)
	{
		set_error(node, KBURN_ERROR_KIND_COMMON, KBurnProtocolOpMismatch, "response invalid: operation not equals");
		goto exit;
	}

	if (response->status != ISP_RET_OK)
	{
		serial_isp_command_error(node, response->status);
		goto exit;
	}

	ret = (void *)node->isp->main_buffer;

exit:
	serial_low_drain_input(node);
	node->serial_isp_busy = false;
	return ret;
}
