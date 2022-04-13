#include "context.h"
#include "basic/lock.h"
#include "device.h"
#include "private-types.h"

void copy_serial_device(kburnDeviceNode *src, kburnDeviceNode *dst) {
	debug_trace_function();

	dst->serial->parent = dst;
	dst->serial->isUsbBound = true;
	dst->serial->deviceInfo = src->serial->deviceInfo;
	strcpy(dst->serial->deviceInfo.path, src->serial->deviceInfo.path);
#ifdef __linux__
	strcpy(dst->serial->deviceInfo.usbDriver, src->serial->deviceInfo.usbDriver);
#endif

	src->disconnect_should_call = false;
	mark_destroy_device_node(src);
}
