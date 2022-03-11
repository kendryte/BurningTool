#pragma once

typedef struct kburnUsbDeviceInfo
{
	PCONST uint16_t idVendor;
	PCONST uint16_t idProduct;
	PCONST uint16_t idRevision;
	PCONST char *driver;

#ifdef __linux__
	PCONST uint32_t major;
	PCONST uint32_t minor;
#endif

} kburnUsbDeviceInfo;

typedef struct kburnUsbDeviceNode
{
	PCONST struct kburnDeviceNode *parent;

	PCONST bool init;

	PCONST bool isConfirm;
	PCONST bool isOpen;

	PCONST kburnUsbDeviceInfo deviceInfo;

} kburnUsbDeviceNode;
