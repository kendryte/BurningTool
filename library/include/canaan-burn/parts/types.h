#pragma once

#include "./errors.h"
#include "./prefix.h"

DEFINE_START

#define KBURN_PROGRAM_BASE_ADDR 0x80080000
typedef uint32_t kburn_mem_address_t;
typedef uint32_t kburn_stor_address_t;
typedef uint32_t kburn_stor_block_t;

typedef struct kburnDeviceError {
	PCONST kburn_err_t code; // (kburnErrorKind << 32) + (slip error code / serial port error code)
	PCONST char *errorMessage;
} kburnDeviceError;

#ifndef has_kb_mutex
typedef struct kb_mutex *kb_mutex_t;
#endif

#include "types.serial.h"
#include "types.usb.h"

typedef struct kburnDeviceNode {
	PCONST kburnDeviceError *const error;
	PCONST void *chipInfo;
	PCONST struct disposable_list *disposable_list;
	kburnSerialDeviceNode *const serial;
	kburnUsbDeviceNode *const usb;
	PCONST struct kburnContext *const _scope;

	PCONST bool destroy_in_progress;
	PCONST bool disconnect_should_call;
	PCONST uint32_t bind_id;
} kburnDeviceNode;

typedef enum kburnLogType {
	KBURN_LOG_BUFFER,
	KBURN_LOG_TRACE,
	KBURN_LOG_DEBUG,
	KBURN_LOG_INFO,
	KBURN_LOG_WARN,
	KBURN_LOG_ERROR,
} kburnLogType;

#define CONCAT(a, b) a##b
#define declare_callback(ret, name, ...)                                                                                                             \
	typedef ret (*name)(void *ctx, __VA_ARGS__);                                                                                                     \
	typedef struct CONCAT(name, _callback_bundle) {                                                                                                  \
		name handler;                                                                                                                                \
		void *context;                                                                                                                               \
	} CONCAT(name, _t)

declare_callback(bool, on_device_connect, const kburnDeviceNode *dev);
declare_callback(void, on_device_remove, const kburnDeviceNode *dev);
declare_callback(void, on_device_handle, kburnDeviceNode *dev);
declare_callback(void, on_write_progress, const kburnDeviceNode *dev, size_t current, size_t length);
declare_callback(void, on_debug_log, kburnLogType type, const char *message);

DEFINE_END
