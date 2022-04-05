#pragma once

#include "./prefix.h"

#define MAX_PATH_LENGTH 8 // 当前usb最大7级，多一个0用于字符串比较
typedef struct kburnUsbDeviceInfo {
	PCONST uint16_t idVendor;
	PCONST uint16_t idProduct;
	PCONST uint8_t path[MAX_PATH_LENGTH];
	PCONST struct libusb_device_descriptor *descriptor;

	PCONST uint8_t endpoint_in;	 /* 端点 in */
	PCONST uint8_t endpoint_out; /* 端点 out */

	// #ifdef __linux__
	// 	PCONST uint32_t major;
	// 	PCONST uint32_t minor;
	// #endif

} kburnUsbDeviceInfo;

typedef struct kburnUsbDeviceNode {
	PCONST struct kburnDeviceNode *parent;

	PCONST bool init;

	PCONST bool isOpen;
	PCONST bool isClaim;

	PCONST kburnUsbDeviceInfo deviceInfo;

	PCONST struct libusb_device *device;
	PCONST struct libusb_device_handle *handle;

	PCONST kb_mutex_t mutex;
} kburnUsbDeviceNode;

typedef struct kburn_usb_device_list {
	PCONST size_t size;
	PCONST kburnSerialDeviceInfo *list;
} * kburnUsbDeviceList;
