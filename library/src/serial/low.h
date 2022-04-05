#pragma once

#include "private-types.h"
#include <stdbool.h>
#include <stdint.h>

bool serial_low_open(kburnSerialDeviceNode *node);
void serial_low_close(kburnSerialDeviceNode *node);
void serial_low_flush_all(kburnSerialDeviceNode *node);
bool serial_low_switch_baudrate(kburnSerialDeviceNode *node, uint32_t br);
int32_t serial_low_drain_input(kburnSerialDeviceNode *node);
void serial_low_output_nulls(kburnSerialDeviceNode *node, bool push_end_char);
isp_response_t *serial_isp_command_send_low_retry(kburnSerialDeviceNode *node, isp_request_t *command, int tries);
