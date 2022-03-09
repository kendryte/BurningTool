#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	enum KBurnSerialError
	{
		KBurnSerialNoErr = 0,
		KBurnSerialDisconnect,
		KBurnSerialFailedOpen,
		KBurnSerialFailedConfirm,
	};

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

	typedef struct kburnSerialNode
	{
		enum KBurnSerialError isError;
		char *errorMessage;
		bool isConfirm;
		bool isOpen;
		const char *path;

		struct ser *m_dev_handle;
	} kburnSerialNode;

#ifdef __cplusplus
}
#endif
