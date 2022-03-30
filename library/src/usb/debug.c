#include "usb.h"

const char *usb_debug_path_string(const uint8_t *path)
{
	static char debug[MAX_PATH_LENGTH * 3 + 1] = "";
	char *debug_itr = debug;
	for (int i = 0; i < MAX_PATH_LENGTH - 1; i++)
	{
		debug_itr += sprintf(debug_itr, "%02x:", path[i]);
	}
	*(debug_itr - 1) = '\0';
	return debug;
}
