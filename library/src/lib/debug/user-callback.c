#include "base.h"
#include "components/call-user-handler.h"
#include "context.h"
#include <stdio.h>

static void default_log_callback(void *UNUSED(context), kburnLogType UNUSED(type), const char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

on_debug_log_t g_on_debug = {
	.handler = default_log_callback,
	.context = NULL,
};

void debug_callback_call(kburnLogType type, const char *message) { CALL_HANDLE_SYNC(g_on_debug, type, message); }

on_debug_log_t kburnSetLogCallback(on_debug_log callback, void *call_context) {
	debug_callback_call(KBURN_LOG_INFO, "logger switched");
	callback_register_swap(on_debug_log, g_on_debug, callback, call_context);
	debug_callback_call(KBURN_LOG_INFO, "logger switched");
	return old;
}
