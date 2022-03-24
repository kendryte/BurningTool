#include "usb.h"

const char *_debug_path_string(const uint8_t *path)
{
	static char debug[MAX_PATH_LENGTH * 6] = "";
	char *debug_itr = debug;
	for (int i = 0; path[i] != 0; i++)
	{
		debug_itr += sprintf(debug, "%d:", path[i]);
	}
	*debug_itr = '\0';
	return debug;
}
