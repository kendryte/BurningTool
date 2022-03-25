#include <Windows.h>

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

bool hackdev_serial_low_switch_baudrate(kburnDeviceNode *node, uint32_t speed)
{
	TODO;
	return false;
}
