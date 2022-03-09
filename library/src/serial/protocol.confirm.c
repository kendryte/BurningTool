#include "serial.h"

bool confirm_port_is_ready(kburnSerialNode *node)
{
	node->isConfirm = true;
	return true;
}
