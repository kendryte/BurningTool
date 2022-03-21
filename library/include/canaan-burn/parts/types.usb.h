#pragma once

#define MAX_SERIAL_LENGTH 256
typedef struct kburnUsbDeviceInfo
{
	PCONST uint16_t idVendor;
	PCONST uint16_t idProduct;
	// PCONST uint16_t idRevision;
	PCONST uint8_t strSerial[MAX_SERIAL_LENGTH];

	// #ifdef __linux__
	// 	PCONST uint32_t major;
	// 	PCONST uint32_t minor;
	// #endif

} kburnUsbDeviceInfo;

typedef struct kburnUsbDeviceNode
{
	PCONST struct kburnDeviceNode *parent;

	PCONST bool init;

	PCONST bool isConfirm;
	PCONST bool isOpen;

	PCONST kburnUsbDeviceInfo deviceInfo;

	PCONST struct libusb_device *device;
	PCONST struct libusb_device_handle *handle;

} kburnUsbDeviceNode;
