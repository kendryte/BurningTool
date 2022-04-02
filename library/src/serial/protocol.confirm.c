#include "serial.h"

bool confirm_port_is_ready(kburnSerialDeviceNode *node) {
	if (!kburnSerialIspGreeting(node)) {
		return false;
	}

	node->isConfirm = true;
	return true;
}
