#include "serial.h"

kburn_err_t on_serial_device_attach(KBCTX scope, const char *path)
{
	DeferEnabled;

	debug_print(GREEN("on_serial_device_attach") "(%s)", path);

	kburnDeviceNode *node = NULL;

	IfErrorReturn(
		create_empty_device_instance(scope, &node));
	DeferDispose(scope->disposables, node, destroy_device);

	IfErrorReturn(
		serial_port_init(node->serial, path));
	dispose_list_add(node->disposable_list, toDisposable(destroy_serial_port, node));

	if (scope->serial->verify_callback != NULL)
	{
		if (!scope->serial->verify_callback(node, scope->serial->verify_callback_ctx))
		{
			debug_print("verify return false");
			return KBURN_ERROR_KIND_COMMON | KBurnUserCancel;
		}
	}

	if (!node->serial->isOpen)
	{
		if (!serial_low_open(node->serial))
		{
			debug_print("open failed");
			return node->error->code;
		}
	}

	if (!node->serial->isConfirm)
	{
		if (!confirm_port_is_ready(node->serial))
		{
			debug_print("confirm failed");
			return node->error->code;
		}
	}

	add_to_device_list(node);

	scope->serial->handler_callback(node, scope->serial->handler_callback_ctx);
	DeferAbort;
	return KBurnNoErr;
}
