#pragma once

#include "context.h"
#include "slip.h"
#include "components/thread.h"
#include "canaan-burn/exported/serial.isp.h"
#include <stdint.h>
#include <stdio.h>
#include <sercomm/sercomm.h>

enum kburnIspOperation
{
	ISP_ECHO = 0xC1,
	ISP_NOP = 0xC2,
	ISP_MEMORY_WRITE = 0xC3,
	ISP_MEMORY_READ = 0xC4,
	ISP_MEMORY_BOOT = 0xC5,
	ISP_UARTHS_BAUDRATE_SET = 0xC6,
	ISP_DEBUG_INFO = 0xD1,
	ISP_FLASH_ERASE = 0xD3,
	ISP_FLASH_WRITE = 0xD4,
	ISP_REBOOT = 0xD5,
	ISP_FLASH_INIT = 0xD7,
	ISP_FLASH_READ = 0xD8,
	ISP_OTP_READ = 0xD9,
	ISP_OTP_WRITE = 0xDA,
	ISP_EMMC_READ = 0xE0,
	ISP_EMMC_WRITE = 0xE1,
	ISP_GET_CHIPID = 0xF0,

	ISP_OP_MAX = UINT8_MAX,
} __attribute__((__packed__));

typedef enum kburnIspOperation kburnIspOperation;
_Static_assert(sizeof(kburnIspOperation) == 1, "enum must 8bit");

struct serial_settings {
	uint8_t retry_times;
	uint32_t command_baudrate;
	uint32_t transfer_baudrate;
	uint32_t isp_baudrate;
	int32_t read_timeout;
	int32_t write_timeout;
	enum KBurnSerialConfigByteSize byte_size;
	enum KBurnSerialConfigParity parity;
	enum KBurnSerialConfigStopBits stop_bits;
};

typedef struct serial_subsystem_context {
	bool subsystem_inited;
	kbthread init_list_thread;

	bool monitor_prepared;
	ser_dev_mon_t *monitor_instance;
	kbthread pairing_thread;
	on_device_connect_t on_verify;
	on_device_handle_t on_handle;

	struct serial_settings settings;
} serial_subsystem_context;

#define SERIAL_CHUNK_SIZE ((size_t)1064)
#define BOARD_MEMORY_PAGE_SIZE ((size_t)1024)
#define MAX_COMMAND_SIZE ((size_t)1024)

typedef struct isp_state {
	slip_descriptor_s descriptor;
	slip_handler_s slip;

	uint8_t main_buffer[MAX_COMMAND_SIZE + 2];
	size_t main_buffer_length;

	uint8_t send_buffer[SERIAL_CHUNK_SIZE];
	size_t send_buffer_length;

	bool has_response;
} isp_state;

typedef struct isp_request_t {
	kburnIspOperation op;
	uint8_t _op_high;
	uint16_t reserved;
	uint32_t checksum;
	kburn_mem_address_t address;
	uint32_t data_len;
	uint8_t data[];
} isp_request_t;

typedef struct isp_response_t {
	kburnIspOperation op;
	kburnIspErrorCode status;
} isp_response_t;

extern uint32_t baudrateHighValue;

#ifdef get_settings
#undef get_settings
#undef subsystem_settings
#undef CREATE_GETTER_SETTER
#else
#define get_settings(node) get_scope(node)->serial->settings
#define subsystem_settings (scope->serial->settings)
#define CREATE_GETTER_SETTER(Field, config_field) _CREATE_GETTER_SETTER(Serial, Field, config_field)
#endif
