#include "serial.h"
#include "usb.h"

static DECALRE_DISPOSE(destroy_device, kburnDeviceNode)
{
	if (context->destroy_in_progress)
		return;
	context->destroy_in_progress = true;
	delete_from_device_list(context);
	destroy_serial_port(context->_scope, context);
	destroy_usb_port(context->_scope, context);
	clear_error(context);
	free(context->error);
	free(context->serial);
	free(context->usb);
	free(context);
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
	kburnDeviceNode n = {
		.error = MyAlloc(kburnDeviceError),
		.chipInfo = NULL,
		.serial = MyAlloc(kburnSerialDeviceNode),
		.usb = MyAlloc(kburnUsbDeviceNode),
		._scope = scope,
		.destroy_in_progress = false,
		.bind_id = 0,
	};

	*output = MyAlloc(kburnDeviceNode);
	memcpy(*output, &n, sizeof(kburnDeviceNode));

	n.serial->parent = *output;
	n.usb->parent = *output;

	add_to_device_list(*output);
	global_resource_register(scope, destroy_device, *output);
	return KBurnNoErr;
}

kburnDeviceNode *kburnOpenSerial(KBCTX scope, const char *path)
{
	debug_print("kburnOpenSerial(%s)", path);
	on_serial_device_attach(scope, path);

	return get_device_by_serial_port_path(scope, path);
}
