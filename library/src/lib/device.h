#pragma once

#include "global.h"

__attribute__((always_inline)) static inline kburnDeviceNode *__p(kburnDeviceNode *v) { return v; }
__attribute__((always_inline)) static inline kburnDeviceNode *__s(kburnSerialDeviceNode *v) { return v->parent; }
__attribute__((always_inline)) static inline kburnDeviceNode *__u(kburnUsbDeviceNode *v) { return v->parent; }
#define get_node(node) _Generic((node), kburnDeviceNode *      \
								: __p, kburnSerialDeviceNode * \
								: __s, kburnUsbDeviceNode *    \
								: __u)(node)

#define serial_isp_command_error(node, err) _serial_isp_command_error(get_node(node), err)
#define copy_last_serial_io_error(node, errno) _copy_last_serial_io_error(get_node(node), errno)
#define set_error(node, kind, code, errstr) _set_error(get_node(node), kind, code, errstr)
#define set_syserr(node) set_error(node, KBURN_ERROR_KIND_SYSCALL, errno, strerror(errno))
#define clear_error(node) _clear_error(get_node(node))
#define error_compare(node, kind, _code) (get_node(node)->error->code == make_error_code(kind, _code))

void _serial_isp_command_error(kburnDeviceNode *node, enum ISPErrorCode err);
void _copy_last_serial_io_error(kburnDeviceNode *node, uint32_t err);
void _set_error(kburnDeviceNode *node, enum kburnErrorKind kind, int32_t code, const char *errstr);
void _clear_error(kburnDeviceNode *node);
kburn_err_t make_error_code(enum kburnErrorKind kind, int32_t code);

void add_to_device_list(kburnDeviceNode *target);
bool delete_from_device_list(kburnDeviceNode *target);
kburnDeviceNode *get_device_by_serial_port_path(KBCTX scope, const char *path);

kburn_err_t create_empty_device_instance(KBCTX scope, kburnDeviceNode **output);
void device_instance_collect(KBCTX scope, kburnDeviceNode *instance);
void device_instance_merge(kburnDeviceNode *dst, kburnDeviceNode *src);
// kburn_err_t set_device_chip_id(kburnDeviceNode *output, chip_id_t chipId);
