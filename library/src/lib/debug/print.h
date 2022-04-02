#pragma once
#include "types.h"
#include "base.h"
#include "basic/string.h"
#include "./low.h"
#include "./level.h"
#include "./path.h"
#include "./user-callback.h"
#include "./color.h"
#include "./assert.h"

void _debug_print(const char *file, int line, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
#define debug_print(level, fmt, ...)                                  \
	if (0)                                                            \
		(void)_debug_print;                                           \
	DEBUG_START(level);                                               \
	_debug_print(__FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__); \
	DEBUG_END()
#define TODO debug_print("\x1B[38;5;11mTODO: please impl %s()\x1B[0m", __func__)

void _print_buffer(const char *file, int line, const char *dir, const uint8_t *buff, size_t size, size_t max_dump);
#define print_buffer(level, dir, buff, size)                \
	if (0)                                                  \
		(void)_print_buffer;                                \
	DEBUG_START(level);                                     \
	_print_buffer(__FILE__, __LINE__, dir, buff, size, 24); \
	DEBUG_END()

#define debug_print_location(level, file, line, fmt, ...)     \
	if (0)                                                    \
		(void)_debug_print;                                   \
	DEBUG_START(level);                                       \
	_debug_print(file, line, fmt __VA_OPT__(, ) __VA_ARGS__); \
	DEBUG_END()

void _debug_trace_function(const char *file, int line, const char *func, const char *fmt, ...) __attribute__((format(printf, 4, 5)));
#define debug_trace_function(...)                                                                                                               \
	if (0)                                                                                                                                      \
		(void)_debug_trace_function;                                                                                                            \
	DEBUG_START(KBURN_LOG_TRACE);                                                                                                               \
	_debug_trace_function(__FILE__, __LINE__, __func__, __VaridicMacro_Opt(__empty_string(""), __VA_ARGS__) __VaridicMacro_Shift(__VA_ARGS__)); \
	DEBUG_END()
