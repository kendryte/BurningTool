#include "serial.h"
#include "usb.h"

DECALRE_DISPOSE(destroy_device, kburnDeviceNode)
{
	if (context->destroy_in_progress)
		return;
	context->destroy_in_progress = true;
	clear_error(context);
	dispose_all(context->disposable_list);
	disposable_list_deinit(context->disposable_list);
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

	add_to_device_list(empty_device_instance);
	dispose_list_add(disposable_list, toDisposable(delete_from_device_list, empty_device_instance));

	*output = empty_device_instance;

	DeferAbort;
	return KBurnNoErr;
}

kburnDeviceNode *kburnOpenSerial(KBCTX scope, const char *path)
{
	debug_print("kburnOpenSerial(%s)", path);
	on_serial_device_attach(scope, path);

	return get_device_by_serial_port_path(scope, path);
}
