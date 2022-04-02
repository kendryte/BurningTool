#include <stdarg.h>
#include <libusb.h>
#include "basic/string.h"
#include "serial.h"

void _copy_last_serial_io_error(kburnDeviceNode *node, uint32_t err)
{
	clear_error(node);
	node->error->code = make_error_code(KBURN_ERROR_KIND_SERIAL, err);
	node->error->errorMessage = strdup(sererr_last());
	debug_print(KBURN_LOG_ERROR, "copy_last_serial_io_error: %s", node->error->errorMessage);
}

void _copy_last_libusb_error(kburnDeviceNode *node, int err)
{
	clear_error(node);
	node->error->code = make_error_code(KBURN_ERROR_KIND_USB, err);
	const char *name = libusb_error_name(err);
	const char *desc = libusb_strerror(err);
	size_t size = m_snprintf(NULL, 0, "%s: %s", name, desc);
	char *buff = malloc(size + 1);
	m_snprintf(buff, size + 1, "%s: %s", name, desc);
	node->error->errorMessage = buff;
	debug_print(KBURN_LOG_INFO, "copy_last_libusb_error: %d - %s", err, node->error->errorMessage);
}

#include <sys/types.h>
typedef union error_convert
{
	struct
	{
#if BYTE_ORDER == LITTLE_ENDIAN
		int32_t code;
		uint32_t kind;
#else
		uint32_t kind;
		int32_t code;
#endif
	} __attribute__((__packed__)) split;
	kburn_err_t combine;
} error_convert;

kburnErrorDesc kburnSplitErrorCode(kburn_err_t code)
{
	error_convert c = {.combine = code};

	return (kburnErrorDesc){
		.code = c.split.code,
		.kind = ((uint64_t)c.split.kind) << 32,
	};
}
kburn_err_t make_error_code(enum kburnErrorKind kind, int32_t code)
{
	error_convert c = {.split = {.kind = kind >> 32, .code = code}};

	return c.combine;
}

void _set_error(kburnDeviceNode *node, enum kburnErrorKind kind, int32_t code, const char *error, ...)
{
	int32_t e = make_error_code(kind, code);
	clear_error(node);
	node->error->code = e;

	va_list args;
	va_start(args, error);
	node->error->errorMessage = vsprintf_alloc(error, args);
	va_end(args);

	debug_print(KBURN_LOG_ERROR, "set_error: %s", node->error->errorMessage);
}

void _clear_error(kburnDeviceNode *node)
{
	node->error->code = 0;
	if (node->error->errorMessage != NULL)
	{
		debug_print(KBURN_LOG_INFO, "clear_error: free last");
		free(node->error->errorMessage);
		node->error->errorMessage = NULL;
	}
}

void slip_error(kburnSerialDeviceNode *node, slip_error_t err)
{
	switch (err)
	{
	case SLIP_ERROR_BUFFER_OVERFLOW:
		set_error(node, KBURN_ERROR_KIND_SLIP, err, "SLIP_ERROR_BUFFER_OVERFLOW");
		break;
	case SLIP_ERROR_UNKNOWN_ESCAPED_BYTE:
		set_error(node, KBURN_ERROR_KIND_SLIP, err, "SLIP_ERROR_UNKNOWN_ESCAPED_BYTE");
		break;
	case SLIP_ERROR_CRC_MISMATCH:
		set_error(node, KBURN_ERROR_KIND_SLIP, err, "SLIP_ERROR_CRC_MISMATCH");
		break;
	default:
		m_abort("invalid slip error");
	}
}

void _serial_isp_command_error(kburnDeviceNode *node, kburnIspErrorCode err)
{
	switch (err)
	{
	case ISP_RET_DEFAULT:
		set_error(node, KBURN_ERROR_KIND_ISP, err, "ISP_RET_DEFAULT");
		return;
	case ISP_RET_BAD_DATA_LEN:
		set_error(node, KBURN_ERROR_KIND_ISP, err, "ISP_RET_BAD_DATA_LEN");
		return;
	case ISP_RET_BAD_DATA_CHECKSUM:
		set_error(node, KBURN_ERROR_KIND_ISP, err, "ISP_RET_BAD_DATA_CHECKSUM");
		return;
	case ISP_RET_INVALID_COMMAND:
		set_error(node, KBURN_ERROR_KIND_ISP, err, "ISP_RET_INVALID_COMMAND");
		return;
	case ISP_RET_INVALID_INITIALIZATION:
		set_error(node, KBURN_ERROR_KIND_ISP, err, "ISP_RET_INVALID_INITIALIZATION");
		return;
	default:
		m_abort("invalid isp command error");
	}
}
