#pragma once

#include "basic/disposable.h"
#include "context.h"
#include "global.h"
#include "serial.isp.h"

DECALRE_DISPOSE_HEADER(destroy_device, kburnDeviceNode);

__attribute__((always_inline)) static inline kburnDeviceNode *__p(kburnDeviceNode *v) { return v; }
__attribute__((always_inline)) static inline kburnDeviceNode *__s(kburnSerialDeviceNode *v) { return v->parent; }
__attribute__((always_inline)) static inline kburnDeviceNode *__u(kburnUsbDeviceNode *v) { return v->parent; }
#define get_node(node) _Generic((node), kburnDeviceNode * : __p, kburnSerialDeviceNode * : __s, kburnUsbDeviceNode * : __u)(node)

#define serial_isp_command_error(node, err) _serial_isp_command_error(get_node(node), err)
#define copy_last_serial_io_error(node, errno) _copy_last_serial_io_error(get_node(node), errno)
#define copy_last_libusb_error(node, errno) _copy_last_libusb_error(get_node(node), errno)
#define set_error(node, kind, code, errstr, ...) _set_error(get_node(node), kind, code, errstr __VA_OPT__(, ) __VA_ARGS__)
#define set_kb_error(node, err, errstr, ...)                                                                                                         \
	set_error(node, kburnSplitErrorCode(err).kind, kburnSplitErrorCode(err).code, errstr __VA_OPT__(, ) __VA_ARGS__)
#define set_syserr(node) set_error(node, KBURN_ERROR_KIND_SYSCALL, errno, strerror(errno))
#define clear_error(node) _clear_error(get_node(node))
#define error_compare(node, kind, _code) (get_node(node)->error->code == make_error_code(kind, _code))

void _serial_isp_command_error(kburnDeviceNode *node, kburnIspErrorCode err);
void _copy_last_serial_io_error(kburnDeviceNode *node, uint32_t err);
void _copy_last_libusb_error(kburnDeviceNode *node, int err);
void _set_error(kburnDeviceNode *node, enum kburnErrorKind kind, int32_t code, const char *errstr, ...) __attribute__((format(printf, 4, 5)));
void _clear_error(kburnDeviceNode *node);

void slip_error(kburnSerialDeviceNode *node, int err);

kburn_err_t create_empty_device_instance(KBCTX scope, kburnDeviceNode **output);
void device_instance_collect(KBCTX scope, kburnDeviceNode *instance);
// kburn_err_t set_device_chip_id(kburnDeviceNode *output, chip_id_t chipId);
