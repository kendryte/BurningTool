#pragma once

DEFINE_START

#define KBURN_PROGRAM_BASE_ADDR 0x80080000
typedef uint32_t kburn_address_t;
typedef uint64_t kburn_err_t;

typedef struct kburnDeviceError
{
	PCONST kburn_err_t code; // (kburnErrorKind << 32) + (slip error code / serial port error code)
	PCONST char *errorMessage;
} kburnDeviceError;

#include "./types.serial.h"
#include "./types.usb.h"

typedef struct kburnDeviceNode
{
	PCONST kburnDeviceError *const error;
	PCONST void *chipInfo;
	kburnSerialDeviceNode *const serial;
	kburnUsbDeviceNode *const usb;
	PCONST struct kburnContext *const _scope;

	PCONST bool destroy_in_progress;
	PCONST uint32_t bind_id;
} kburnDeviceNode;

typedef bool (*on_device_connect)(const kburnDeviceNode *dev, void *ctx);
typedef void (*on_device_remove)(const kburnDeviceNode *dev, void *ctx);
typedef void (*on_device_handle)(kburnDeviceNode *dev, void *ctx);
typedef void (*on_write_progress)(const kburnDeviceNode *dev, size_t current, size_t length, void *ctx);

DEFINE_END
