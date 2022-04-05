#include "basic/lock.h"
#include "device.h"
#include "private-types.h"
#include "types.h"

void copy_serial_device(kburnDeviceNode *src, kburnDeviceNode *dst) {
	dst->serial->parent = dst;
	dst->serial->isUsbBound = true;
	dst->serial->deviceInfo = src->serial->deviceInfo;
	dst->serial->deviceInfo.path = strdup(src->serial->deviceInfo.path);
#ifdef __linux__
	if (src->serial->deviceInfo.usbDriver)
		dst->serial->deviceInfo.usbDriver = strdup(src->serial->deviceInfo.usbDriver);
#endif

	src->disconnect_should_call = false;
	destroy_device(src->_scope->disposables, src);
}
