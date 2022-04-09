#pragma once

#include "base.h"
#include "context.h"
#include "canaan-burn/exported/types.h"
#include <stdarg.h>

void _kb__debug_enter();
void _kb__debug_leave(kburnLogType level);
extern char *const debug_output;
extern const size_t debug_buffer_remain;
char *debug_output_move(size_t size);
void debug_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void debug_vprintf(const char *fmt, va_list va);
void debug_puts(const char *message);

#define DEBUG_START(level)                  \
	if (debug_check_level(level)) {         \
		kburnLogType s_debug_level = level; \
		_kb__debug_enter();

#define DEBUG_END()                  \
	_kb__debug_leave(s_debug_level); \
	}

#define DEBUG_FORMAT_INPUT debug_output, debug_buffer_remain
