#include "serial.h"
#include "driver.h"

static inline void free_handle(void *handle)
{
	if (handle)
		free(handle);
}

DECALRE_DISPOSE(destroy_serial_port, kburnDeviceNode)
{
	kburnSerialDeviceNode *serial = context->serial;
	if (!serial->init)
		return;

	lock(serial->mutex);

	debug_print("destroy_serial_port(%p[%s])", (void *)serial, serial->path);

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

	unlock(serial->mutex);
	lock_deinit(&serial->mutex);
	device_instance_collect(context->_scope, context);
}
DECALRE_DISPOSE_END()

kburn_err_t serial_port_init(kburnSerialDeviceNode *serial, const char *path)
{
	m_assert(!serial->init, "serial port must not already inited");

	debug_print("serial_port_init(0x%p, %s)", (void *)serial, path);
	serial->path = CheckNull(strdup(path));
	serial->deviceInfo = driver_get_devinfo(path);
	serial->mutex = CheckNull(lock_init());

	serial_isp_open(serial);

	serial->init = true;

	return KBurnNoErr;
}
