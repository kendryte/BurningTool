#include "./level.h"

bool g_debug_dump_buffer_enabled = false;

bool debug_check_level(kburnLogType level) {
	return (level != KBURN_LOG_BUFFER || g_debug_dump_buffer_enabled);
}

void kburnSetLogBufferEnabled(bool enable) {
	g_debug_dump_buffer_enabled = enable;
}
