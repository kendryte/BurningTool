#pragma once

#include <stdint.h>
#include <stdio.h>

#include "global.h"

#include <sercomm/sercomm.h>
#include "slip.h"

ser_opts_t get_current_serial_options(const char *path);

typedef struct serial_subsystem_context
{
	ser_dev_mon_t *monitor_instance;
	on_device_connect verify_callback;
	void *verify_callback_ctx;
	on_device_handle handler_callback;
	void *handler_callback_ctx;
	on_device_remove disconnect_callback;
	void *disconnect_callback_ctx;
} serial_subsystem_context;

#define SERIAL_CHUNK_SIZE 1064
#define BOARD_MEMORY_PAGE_SIZE 1024
#define MAX_COMMAND_SIZE 1024

typedef struct isp_state
{
	slip_descriptor_s descriptor;
	slip_handler_s slip;

	uint8_t main_buffer[MAX_COMMAND_SIZE + 2];
	size_t main_buffer_length;

	uint8_t send_buffer[SERIAL_CHUNK_SIZE];
	size_t send_buffer_length;

	bool has_response;
} isp_state;

enum ISPOperation
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

typedef struct isp_request_t
{

	enum ISPOperation op;
	uint8_t _op_high;
	uint16_t reserved;
	uint32_t checksum;
	kburn_address_t address;
	uint32_t data_len;
	uint8_t data[1024];
} isp_request_t;

typedef struct isp_response_t
{

	enum ISPOperation op;
	enum ISPErrorCode status;
} isp_response_t;

extern uint32_t baudrateHighValue;

kburn_err_t init_serial_port(kburnSerialDeviceNode *serial, const char *path);
void destroy_serial_port(KBCTX scope, kburnDeviceNode *node);
kburn_err_t on_serial_device_attach(KBCTX scope, const char *path);

bool serial_low_open(kburnSerialDeviceNode *node);
void serial_low_close(kburnSerialDeviceNode *node);
void serial_low_flush_all(kburnSerialDeviceNode *node);
bool serial_low_switch_baudrate(kburnSerialDeviceNode *node, uint32_t br);
int32_t serial_low_drain_input(kburnSerialDeviceNode *node);
void serial_low_output_nulls(kburnSerialDeviceNode *node, bool push_end_char);

bool hackdev_serial_low_switch_baudrate(kburnSerialDeviceNode *node, uint32_t br);

bool confirm_port_is_ready(kburnSerialDeviceNode *node);

void serial_isp_open(kburnSerialDeviceNode *node);
void serial_isp_delete(kburnSerialDeviceNode *node);

isp_response_t *serial_isp_command_send_low_retry(kburnSerialDeviceNode *node, isp_request_t *command, int tries);
isp_response_t *serial_isp_command_send_low(kburnSerialDeviceNode *node, isp_request_t *command);

isp_request_t *new_serial_isp_packet(uint32_t data_length);
bool serial_isp_verify_checksum(isp_request_t *packet);
void serial_isp_calculate_checksum(isp_request_t *packet);
size_t serial_isp_packet_size(const isp_request_t *packet);
#define make_serial_isp_packet(pointer_name, data_length)                          \
	size_t __isp_packet_memory_size = offsetof(isp_request_t, data) + data_length; \
	uint8_t __isp_packet_memory[__isp_packet_memory_size];                         \
	memset(__isp_packet_memory, 0, __isp_packet_memory_size);                      \
	isp_request_t *pointer_name = (void *)__isp_packet_memory;                     \
	pointer_name->data_len = data_length;                                          \
	debug_print("[alloc] %ld bytes, data len=%d", __isp_packet_memory_size, pointer_name->data_len)

void slip_error(kburnSerialDeviceNode *node, slip_error_t err);

kburn_err_t serial_subsystem_init(KBCTX scope);
void serial_subsystem_deinit(KBCTX scope);
void serial_monitor_pause(KBCTX scope);
kburn_err_t serial_monitor_resume(KBCTX scope);
