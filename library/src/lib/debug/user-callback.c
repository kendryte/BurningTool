#include "base.h"
#include "types.h"
#include <stdio.h>
#include "canaan-burn/canaan-burn.h"

static void default_log_callback(void *UNUSED(context), kburnLogType UNUSED(type), const char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

on_debug_log g_on_debug_callback = default_log_callback;
void *g_on_debug_callback_ctx = NULL;

struct debug_callback kburnSetLogCallback(on_debug_log callback, void *call_context)
{
	g_on_debug_callback(g_on_debug_callback_ctx, KBURN_LOG_INFO, "logger switched");
	struct debug_callback old = {.callback = g_on_debug_callback, .call_context = g_on_debug_callback_ctx};
	g_on_debug_callback = callback;
	g_on_debug_callback_ctx = call_context;
	g_on_debug_callback(g_on_debug_callback_ctx, KBURN_LOG_INFO, "logger switched");
	return old;
}
