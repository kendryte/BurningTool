#include "usb.h"

void kburnSetUsbFilter(KBCTX scope, int vid, int pid)
{
	if (vid == KBURN_VIDPID_FILTER_DEFAULT)
	{
		scope->usb->filter.vid = DEFAULT_VID;
	}
	else
	{
		scope->usb->filter.vid = vid;
	}

	if (pid == KBURN_VIDPID_FILTER_DEFAULT)
	{
		scope->usb->filter.pid = DEFAULT_PID;
	}
	else
	{
		scope->usb->filter.pid = pid;
	}
}
