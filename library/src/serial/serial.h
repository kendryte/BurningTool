#pragma once

#include <stdint.h>
#include <stdio.h>

#include "global.h"
#include "thread.h"

#include <sercomm/sercomm.h>
#include "slip.h"
#include "isp.h"

ser_opts_t get_current_serial_options(const char *path);

typedef struct serial_subsystem_context
{
	bool subsystem_inited;
	kbthread init_list_thread;

	bool monitor_prepared;
	ser_dev_mon_t *monitor_instance;
	kbthread pairing_thread;
	on_device_connect verify_callback;
	void *verify_callback_ctx;
	on_device_handle handler_callback;
	void *handler_callback_ctx;
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

typedef struct isp_request_t
{
	kburnIspOperation op;
	uint8_t _op_high;
	uint16_t reserved;
	uint32_t checksum;
	kburn_mem_address_t address;
	uint32_t data_len;
	uint8_t data[];
} isp_request_t;

typedef struct isp_response_t
{

	kburnIspOperation op;
	kburnIspErrorCode status;
} isp_response_t;

extern uint32_t baudrateHighValue;

kburn_err_t serial_port_init(kburnSerialDeviceNode *serial, const char *path);
DECALRE_DISPOSE_HEADER(destroy_serial_port, kburnDeviceNode);
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

kburn_err_t serial_monitor_prepare(KBCTX scope);
void serial_monitor_destroy(KBCTX scope);
void serial_monitor_pause(KBCTX scope);
kburn_err_t serial_monitor_resume(KBCTX scope);
