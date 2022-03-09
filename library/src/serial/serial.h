#pragma once

#include <stdint.h>
#include <stdio.h>

#include "global.h"

#include <sercomm/sercomm.h>

int32_t serial_open_with_current_config(ser_t *ser, const char *path);

typedef struct monitor
{
	ser_dev_mon_t *instance;
	on_device_connect verify_callback;
	on_device_handle handler_callback;
} monitor;

extern monitor mon;

kburnSerialNode *create_port(const char *path);
void destroy_port(kburnSerialNode *node);

void on_device_attach(const char *path);

bool serial_low_open(kburnSerialNode *node);
void copy_last_error(kburnSerialNode *node);
bool confirm_port_is_ready(kburnSerialNode *node);

void add_to_port_list(kburnSerialNode *target);
bool pop_from_port_list(kburnSerialNode *target);
kburnSerialNode *find_from_port_list(const char *path);
