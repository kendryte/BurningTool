#include "serial.h"

kburn_err_t on_serial_device_attach(KBCTX scope, const char *path)
{
	kburn_err_t err;
	debug_print("on_serial_device_attach(%s)", path);

	kburnDeviceNode *node;

	err = create_empty_device_instance(scope, &node);
	m_assert_err(err, "critical memory alloc failed");
	if (err != KBurnNoErr)
		return err;

	err = serial_port_init(node->serial, path);
	m_assert_err(err, "serial_port_init now do not return error");
	if (err != KBurnNoErr)
		return err;

	if (scope->serial->verify_callback != NULL)
	{
		if (!scope->serial->verify_callback(node, scope->serial->verify_callback_ctx))
		{
			debug_print("verify return false");
			destroy_serial_port(scope, node);
			return KBURN_ERROR_KIND_COMMON | KBurnUserCancel;
		}
	}

	if (!node->serial->isOpen)
	{
		if (!serial_low_open(node->serial))
		{
			err = node->error->code;
			debug_print("open failed");
			destroy_serial_port(scope, node);
			return err;
		}
	}

	if (!node->serial->isConfirm)
	{
		if (!confirm_port_is_ready(node->serial))
		{
			err = node->error->code;
			debug_print("confirm failed");
			destroy_serial_port(scope, node);
			return err;
		}
	}

	scope->serial->handler_callback(node, scope->serial->handler_callback_ctx);
	return err;
}
