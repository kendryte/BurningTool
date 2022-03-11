#pragma once

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

typedef struct kburnSerialDeviceInfo
{
	PCONST bool isUSB;
	PCONST bool isTTY;

	PCONST uint16_t usbIdVendor;
	PCONST uint16_t usbIdProduct;
	PCONST uint16_t usbIdRevision;
	PCONST char *usbDriver;

#ifdef __linux__
	PCONST uint32_t major;
	PCONST uint32_t minor;
#endif

} kburnSerialDeviceInfo;

typedef struct kburnSerialDeviceNode
{
	PCONST struct kburnDeviceNode *parent;

	PCONST bool init;

	PCONST bool isConfirm;
	PCONST bool isOpen;
	const char *path;

	PCONST uint32_t baudRate;
	PCONST bool serial_isp_busy;

	PCONST kburnSerialDeviceInfo deviceInfo;

	PCONST struct isp_state *isp;
	PCONST struct ser *m_dev_handle;
} kburnSerialDeviceNode;
