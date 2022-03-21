#pragma once

DEFINE_START

enum KBurnCommonError
{
	KBurnNoErr = 0,
	KBurnNoMemory,
	KBurnSerialDriverAttrReadErr,
	KBurnSerialDriverAttrWriteErr,
	KBurnSerialDriverUnsupportChange,
	KBurnSerialMonitorFailStart,
	KBurnSerialFailListDevice,
	KBurnUsbMonitorFailStart,
	KBurnUsbDeviceNotFound,
	KBurnProtocolOpMismatch,
	KBurnUserCancel,
};

#pragma GCC diagnostic ignored "-Wpedantic"
enum kburnErrorKind
{
	KBURN_ERROR_KIND_COMMON = 0UL,
	KBURN_ERROR_KIND_SERIAL = 1UL << 32,
	KBURN_ERROR_KIND_SLIP = 1UL << 33,
	KBURN_ERROR_KIND_ISP = 1UL << 34,
	KBURN_ERROR_KIND_SYSCALL = 1UL << 35,
	KBURN_ERROR_KIND_USB = 1ULL << 36,

	KBURN_ERROR_KIND_MAX = UINT64_MAX,
};
#pragma GCC diagnostic pop

typedef struct kburnErrorDesc
{
	enum kburnErrorKind kind;
	int32_t code;
} __attribute__((__packed__)) kburnErrorDesc;
kburnErrorDesc kburnSplitErrorCode(kburn_err_t code);

static_assert(sizeof(kburnErrorDesc) == sizeof(uint64_t) + sizeof(int32_t), "kburnErrorDesc must be 96bit");

DEFINE_END
