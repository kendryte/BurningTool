#include "serial.h"
#include "usb.h"
#include "components/device-link-list.h"

void kburnOnDeviceDisconnect(KBCTX scope, on_device_remove callback, void *ctx)
{
	scope->disconnect_callback = callback;
	scope->disconnect_callback_ctx = ctx;
}

DECALRE_DISPOSE(destroy_device, kburnDeviceNode)
{
	if (context->destroy_in_progress)
		return;
	context->destroy_in_progress = true;

	if (context->disconnect_should_call && context->_scope->disconnect_callback)
	{
		debug_print(KBURN_LOG_DEBUG, "\tdisconnect_callback()");
		context->_scope->disconnect_callback(context, context->_scope->disconnect_callback_ctx);
	}

	clear_error(context);
	disposable_list_t *dlist = context->disposable_list;
	dispose_all(dlist);
	disposable_list_deinit(dlist);
}
DECALRE_DISPOSE_END()

void device_instance_collect(KBCTX scope, kburnDeviceNode *instance)
{
	if (!instance->serial->init && !instance->usb->init && !instance->destroy_in_progress)
	{
		destroy_device(scope->disposables, instance);
	}
}

kburn_err_t create_empty_device_instance(KBCTX scope, kburnDeviceNode **output)
{
	DeferEnabled;

	disposable_list_t *disposable_list = DeferFree(CheckNull(disposable_list_init("device instance")));
	DeferCall(disposable_list_deinit, disposable_list);
	DeferCall(dispose_all, disposable_list);

	kburnDeviceError *error = MyAlloc(kburnDeviceError);
	register_dispose_pointer(disposable_list, error);

	kburnSerialDeviceNode *serial = MyAlloc(kburnSerialDeviceNode);
	register_dispose_pointer(disposable_list, serial);

	kburnUsbDeviceNode *usb = MyAlloc(kburnUsbDeviceNode);
	register_dispose_pointer(disposable_list, usb);

	kburnDeviceNode n = {
		.disposable_list = disposable_list,
		.error = error,
		.chipInfo = NULL,
		.serial = serial,
		.usb = usb,
		._scope = scope,
		.destroy_in_progress = false,
		.bind_id = 0,
	};

	kburnDeviceNode *empty_device_instance = MyAlloc(kburnDeviceNode);
	register_dispose_pointer(disposable_list, empty_device_instance);

	memcpy(empty_device_instance, &n, sizeof(kburnDeviceNode));

	n.serial->parent = empty_device_instance;
	n.usb->parent = empty_device_instance;

	*output = empty_device_instance;

	DeferAbort;
	return KBurnNoErr;
}

kburnDeviceNode *kburnOpenSerial(KBCTX scope, const char *path)
{
	debug_print(KBURN_LOG_DEBUG, "kburnOpenSerial(%s)", path);
	on_serial_device_attach(scope, path);

	return get_device_by_serial_port_path(scope, path);
}
