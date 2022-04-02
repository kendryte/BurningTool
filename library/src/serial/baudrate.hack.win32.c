#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "base.h"
#include "debug/print.h"
#include "types.h"

// https://github.com/ingeniamc/sercomm/blob/master/include/sercomm/win/types.h
struct ser
{
	HANDLE hnd;
	DCB dcb_old;
	COMMTIMEOUTS timeouts_old;
	struct
	{
		DWORD rd;
		DWORD wr;
	} timeouts;
};

bool hackdev_serial_low_switch_baudrate(kburnDeviceNode *UNUSED(node), uint32_t UNUSED(speed))
{
	TODO;
	return false;
}
