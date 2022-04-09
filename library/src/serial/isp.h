#pragma once

#include "private-types.h"

void serial_isp_open(kburnSerialDeviceNode *node);
void serial_isp_delete(kburnSerialDeviceNode *node);
void serial_isp_calculate_checksum(isp_request_t *packet);
size_t serial_isp_packet_size(const isp_request_t *packet);

#define make_serial_isp_packet(pointer_name, data_length)                          \
	if (0)                                                                         \
		(void)(0);                                                                 \
	size_t __isp_packet_memory_size = offsetof(isp_request_t, data) + data_length; \
	uint8_t __isp_packet_memory[__isp_packet_memory_size];                         \
	memset(__isp_packet_memory, 0, __isp_packet_memory_size);                      \
	isp_request_t *pointer_name = (void *)__isp_packet_memory;                     \
	pointer_name->data_len = data_length;                                          \
	debug_print(KBURN_LOG_TRACE, "[alloc] " FMT_SIZET " bytes, data len=%u", __isp_packet_memory_size, pointer_name->data_len)
