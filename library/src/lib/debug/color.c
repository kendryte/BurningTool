#include "debug.h"
#include "debug/print.h"

kburnDebugColors g_debug_colors = {
	.red = {.prefix = "\x1B[38;5;9m", .postfix = "\x1B[0m"},
	.green = {.prefix = "\x1B[38;5;10m", .postfix = "\x1B[0m"},
	.yellow = {.prefix = "\x1B[38;5;11m", .postfix = "\x1B[0m"},
	.grey = {.prefix = "\x1B[2m", .postfix = "\x1B[0m"},
};

kburnDebugColors kburnSetColors(kburnDebugColors color_list)
{
	kburnDebugColors old = g_debug_colors;
	old = color_list;
	return old;
}
