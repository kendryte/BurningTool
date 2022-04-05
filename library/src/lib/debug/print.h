#pragma once
#include "./assert.h"
#include "./color.h"
#include "./level.h"
#include "./low.h"
#include "./path.h"
#include "./user-callback.h"
#include "base.h"
#include "basic/string.h"
#include "context.h"

#define debug_print(level, fmt, ...) debug_print_location(level, __FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define TODO debug_print("\x1B[38;5;11mTODO: please impl %s()\x1B[0m", __func__)

size_t __print_buffer(char *output, size_t output_length, const char *dir, const uint8_t *buff, size_t size, size_t max_dump);
#define print_buffer(level, dir, buff, size)                                                                                                         \
	if (0)                                                                                                                                           \
		(void)__print_buffer;                                                                                                                        \
	DEBUG_START(level);                                                                                                                              \
	debug_print_prefix(debug_output, debug_buffer_remain, __FILE__, __LINE__);                                                                       \
	debug_output_move(__print_buffer(debug_output, debug_buffer_remain, dir, buff, size, 24));                                                       \
	DEBUG_END()

#define debug_print_location(level, file, line, fmt, ...)                                                                                            \
	if (0)                                                                                                                                           \
		(void)0;                                                                                                                                     \
	DEBUG_START(level);                                                                                                                              \
	debug_print_prefix(debug_output, debug_buffer_remain, file, line);                                                                               \
	debug_printf(fmt __VA_OPT__(, ) __VA_ARGS__);                                                                                                    \
	DEBUG_END()

#ifndef NDEBUG
#define INCLUDE_FILE_LINE 1
#else
#define INCLUDE_FILE_LINE 0
#endif

#define debug_trace_function(...)                                                                                                                    \
	if (0)                                                                                                                                           \
		(void)0;                                                                                                                                     \
	DEBUG_START(KBURN_LOG_TRACE);                                                                                                                    \
	debug_print_prefix(debug_output, debug_buffer_remain, __FILE__, __LINE__);                                                                       \
	if (INCLUDE_FILE_LINE)                                                                                                                           \
		debug_printf("[%.*s] ", basename_to_ext_length(__FILE__), basename(__FILE__));                                                               \
	debug_printf(COLOR_FMT("%s") "(", COLOR_ARG(GREY, __func__));                                                                                    \
	debug_printf(__VaridicMacro_Opt(__clang_happy_fmt_string(""), __VA_ARGS__) __VaridicMacro_Shift(__VA_ARGS__));                                   \
	debug_printf(")");                                                                                                                               \
	DEBUG_END()
