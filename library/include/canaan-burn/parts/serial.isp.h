#pragma once

#include "./prefix.h"

DEFINE_START

enum kburnIspErrorCode
{
	ISP_RET_DEFAULT = 0,
	ISP_RET_OK = 0xE0,
	ISP_RET_BAD_DATA_LEN = 0xE1,
	ISP_RET_BAD_DATA_CHECKSUM = 0xE2,
	ISP_RET_INVALID_COMMAND = 0xE3,
	ISP_RET_INVALID_INITIALIZATION = 0xE4,

	ISP_RET_MAX = UINT8_MAX,
} __attribute__((__packed__));
typedef enum kburnIspErrorCode kburnIspErrorCode;

PUBLIC bool kburnSerialIspGreeting(kburnSerialDeviceNode *node);
PUBLIC bool kburnSerialIspSetBaudrate(kburnSerialDeviceNode *node, uint32_t want_br);
PUBLIC bool kburnSerialIspSetBaudrateHigh(kburnSerialDeviceNode *node);
PUBLIC bool kburnSerialIspMemoryWrite(kburnSerialDeviceNode *node, kburn_mem_address_t address, const char *data, size_t data_len, on_write_progress page_callback, void *ctx);
PUBLIC bool kburnSerialIspBootMemory(kburnSerialDeviceNode *node, kburn_mem_address_t address);

PUBLIC bool kburnSerialIspRunProgram(kburnSerialDeviceNode *node, const void *programBuffer, size_t programBufferSize, on_write_progress page_callback, void *ctx);
PUBLIC bool kburnSerialIspSwitchUsbMode(kburnSerialDeviceNode *node, on_write_progress page_callback, void *ctx);

DEFINE_END
