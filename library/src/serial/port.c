#include "driver.h"
#include "serial.h"

static inline void free_handle(void *handle) {
	if (handle)
		free(handle);
}

DECALRE_DISPOSE(destroy_serial_port, kburnDeviceNode) {
	kburnSerialDeviceNode *serial = context->serial;
	debug_trace_function("%p[%s]", (void *)serial, OPTSTR(serial->path, "*invalid*"));

	if (!serial->init)
		return;

	lock(serial->mutex);

	serial_isp_delete(serial);

	if (serial->binding) {
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

kburn_err_t serial_port_init(kburnSerialDeviceNode *serial, const char *path) {
	debug_trace_function("0x%p, %s", (void *)serial, path);

	m_assert(!serial->init, "serial port must not already inited");

	serial->path = CheckNull(strdup(path));
	serial->deviceInfo = driver_get_devinfo(path);
	serial->mutex = CheckNull(lock_init());

	serial_isp_open(serial);

	serial->init = true;

	return KBurnNoErr;
}
