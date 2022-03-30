#pragma once

#include "./prefix.h"

#define MAX_SERIAL_LENGTH 256
#define MAX_PATH_LENGTH 8 // 当前usb最大7级，多一个0用于字符串比较
typedef struct kburnUsbDeviceInfo
{
	PCONST uint16_t idVendor;
	PCONST uint16_t idProduct;
	// PCONST uint16_t idRevision;
	PCONST uint8_t strSerial[MAX_SERIAL_LENGTH];
	PCONST uint8_t path[MAX_PATH_LENGTH];

	PCONST uint8_t endpoint_in;	 /* 端点 in */
	PCONST uint8_t endpoint_out; /* 端点 out */

	// #ifdef __linux__
	// 	PCONST uint32_t major;
	// 	PCONST uint32_t minor;
	// #endif

} kburnUsbDeviceInfo;

typedef struct kburnUsbDeviceNode
{
	PCONST struct kburnDeviceNode *parent;

	PCONST bool init;

	PCONST bool isOpen;
	PCONST bool isClaim;

	PCONST kburnUsbDeviceInfo deviceInfo;

	PCONST struct libusb_device *device;
	PCONST struct libusb_device_handle *handle;

	PCONST kb_mutex_t mutex;
} kburnUsbDeviceNode;
