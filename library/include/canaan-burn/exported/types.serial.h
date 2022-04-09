#pragma once

#include "./prefix.h"
#include "./types.h"

DEFINE_START

enum KBurnSerialConfigByteSize
{
	KBurnSerialConfigByteSize_8,
	KBurnSerialConfigByteSize_7,
	KBurnSerialConfigByteSize_6,
	KBurnSerialConfigByteSize_5
};

enum KBurnSerialConfigParity
{
	KBurnSerialConfigParityNone,
	KBurnSerialConfigParityOdd,
	KBurnSerialConfigParityEven,
	KBurnSerialConfigParityMark,
	KBurnSerialConfigParitySpace
};

enum KBurnSerialConfigStopBits
{
	KBurnSerialConfigStopBitsOne,
	KBurnSerialConfigStopBitsOneHalf,
	KBurnSerialConfigStopBitsTwo
};

#define MAX_SERIAL_PATH_SIZE 256
#define MAX_DRIVER_NAME_SIZE 32
#define MAX_TITTLE_SIZE 256
#define MAX_WIN32_PATH_SIZE 256

struct kburnSerialDeviceInfoSlice {
	PCONST bool isUSB;
	PCONST bool isTTY;
	PCONST char path[MAX_SERIAL_PATH_SIZE];

	PCONST uint16_t usbIdVendor;
	PCONST uint16_t usbIdProduct;
	PCONST uint16_t usbIdRevision;

#ifdef __linux__
	PCONST char usbDriver[MAX_DRIVER_NAME_SIZE];
	PCONST uint32_t deviceMajor;
	PCONST uint32_t deviceMinor;
#endif
#ifdef WIN32
	PCONST char title[MAX_TITTLE_SIZE];
	PCONST char hwid[MAX_TITTLE_SIZE];
#endif
};

typedef struct kburnSerialDeviceInfo {
	PCONST bool isUSB;
	PCONST bool isTTY;
	PCONST char path[MAX_SERIAL_PATH_SIZE];

	PCONST uint16_t usbIdVendor;
	PCONST uint16_t usbIdProduct;
	PCONST uint16_t usbIdRevision;

#ifdef __linux__
	PCONST char usbDriver[MAX_DRIVER_NAME_SIZE];
	PCONST uint32_t deviceMajor;
	PCONST uint32_t deviceMinor;
#endif
#ifdef WIN32
	PCONST char title[MAX_TITTLE_SIZE];
	PCONST char hwid[MAX_TITTLE_SIZE];
#endif

} kburnSerialDeviceInfo;

typedef struct kburnSerialDeviceNode {
	PCONST struct kburnDeviceNode *parent;

	PCONST bool init;

	PCONST bool isConfirm;
	PCONST bool isOpen;
	PCONST bool isSwitchIsp;
	PCONST bool isUsbBound;

	PCONST uint32_t baudRate;
	PCONST bool serial_isp_busy;

	PCONST kburnSerialDeviceInfo deviceInfo;

	PCONST struct isp_state *isp;
	PCONST struct binding_state *binding;
	PCONST struct ser *m_dev_handle;

	PCONST kb_mutex_t mutex;
} kburnSerialDeviceNode;

typedef struct kburn_serial_device_list {
	PCONST size_t size;
	PCONST struct kburnSerialDeviceInfoSlice *list;
} kburnSerialDeviceList;

DEFINE_END
