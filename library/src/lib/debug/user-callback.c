#include "base.h"
#include "types.h"
#include <stdio.h>

static void default_log_callback(kburnLogType UNUSED(type), const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

on_debug_log g_on_debug_callback = default_log_callback;

on_debug_log kburnSetLogCallback(on_debug_log callback)
{
	on_debug_log old = g_on_debug_callback;
	g_on_debug_callback = callback;
	return old;
}
