#include "serial.h"

void _copy_last_serial_io_error(kburnDeviceNode *node, uint32_t err)
{
	clear_error(node);
	node->error->code = err | KBURN_ERROR_KIND_SERIAL;
	node->error->errorMessage = strdup(sererr_last());
	debug_print("copy_last_serial_io_error: %s", node->error->errorMessage);
}

kburnErrorDesc kburnSplitErrorCode(kburn_err_t code)
{
	kburnErrorDesc a = {
		.kind = code & ~(uint64_t)UINT32_MAX,
		.code = code & UINT32_MAX,
	};
	return a;
}
kburn_err_t make_error_code(enum kburnErrorKind kind, int32_t code)
{
	return kind | code;
}

void _set_error(kburnDeviceNode *node, enum kburnErrorKind kind, int32_t code, const char *error)
{
	int32_t e = make_error_code(kind, code);
	clear_error(node);
	node->error->code = e;
	node->error->errorMessage = strdup(error);
	debug_print("set_error: %s", node->error->errorMessage);
}

void _clear_error(kburnDeviceNode *node)
{
	node->error->code = 0;
	if (node->error->errorMessage != NULL)
	{
		debug_print("clear_error: free last");
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
		assert("invalid slip error");
	}
}

void _serial_isp_command_error(kburnDeviceNode *node, enum ISPErrorCode err)
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
		assert("invalid isp command error");
	}
}
