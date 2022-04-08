#pragma once

#include "private-types.h"
#include <stdbool.h>
#include <stdint.h>

bool serial_low_open(kburnSerialDeviceNode *node);
void serial_low_close(kburnSerialDeviceNode *node);
void serial_low_flush_all(kburnSerialDeviceNode *node);
bool serial_low_switch_baudrate(kburnSerialDeviceNode *node, uint32_t br);
bool serial_low_write_data(kburnSerialDeviceNode *node, const void *data, size_t data_length, size_t *written);
bool serial_low_read_data(kburnSerialDeviceNode *node, uint8_t **buffer, size_t *buffer_size, size_t *read);
bool serial_low_drain_input(kburnSerialDeviceNode *node, size_t *drop_bytes);
void serial_low_output_nulls(kburnSerialDeviceNode *node, bool push_end_char);
isp_response_t *serial_isp_command_send_low_retry(kburnSerialDeviceNode *node, isp_request_t *command, int tries);
