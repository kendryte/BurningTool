#include "basic/lock.h"
#include "types.h"
#include "serial.h"

void copy_serial_device(kburnDeviceNode *src, kburnDeviceNode *dst)
{
	dst->serial->parent = dst;
	dst->serial->isUsbBound = true;
	dst->serial->path =
		register_dispose_pointer(dst->disposable_list, strdup(src->serial->path));

	src->disconnect_should_call = false;
	destroy_device(src->_scope->disposables, src);
}
