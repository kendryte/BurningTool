#include "serial.h"

void kburnOnSerialDisconnect(KBCTX scope, on_device_remove callback, void *ctx)
{
	scope->serial->disconnect_callback = callback;
	scope->serial->disconnect_callback_ctx = ctx;
}

static inline void free_handle(void *handle)
{
	if (handle)
		free(handle);
}

void destroy_serial_port(KBCTX scope, kburnDeviceNode *d)
{
	kburnSerialDeviceNode *serial = d->serial;
	if (!serial->init)
		return;

	debug_print("destroy_serial_port(%p[%s])", (void *)serial, serial->path);

	if (scope->serial->disconnect_callback)
	{
		debug_print("\tdisconnect_callback()");
		scope->serial->disconnect_callback(d, scope->serial->disconnect_callback_ctx);
	}

	serial_isp_delete(serial);

	if (serial->binding)
	{
		free(serial->binding);
		serial->binding = NULL;
	}

	if (serial->isOpen)
		serial_low_close(serial);

	free_handle((void *)serial->path);

	if (serial->m_dev_handle != NULL)
		ser_destroy(serial->m_dev_handle);

	driver_get_devinfo_free(serial->deviceInfo);
	serial->init = false;

	device_instance_collect(scope, d);
}

kburn_err_t init_serial_port(kburnSerialDeviceNode *serial, const char *path)
{
	debug_print("init_serial_port(0x%p, %s)", (void *)serial, path);
	serial->path = strdup(path);
	serial->deviceInfo = driver_get_devinfo(path);

	serial_isp_open(serial);

	serial->init = true;

	return KBurnNoErr;
}
