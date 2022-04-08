#pragma once
#include "base.h"
#include "context.h"
#include "./assert.h"
#include "./color.h"
#include "./level.h"
#include "./low.h"
#include "./path.h"
#include "./user-callback.h"
#include "basic/string.h"
#include <inttypes.h>

#define debug_print(level, fmt, ...) debug_print_location(level, __FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define TODO debug_print(KBURN_LOG_ERROR, COLOR_FMT("TODO: please impl %s()"), COLOR_ARG(RED, __func__))

#define PRINT_BUFF_MAX 24
size_t _kb__print_buffer(char *output, size_t output_length, const char *dir, const uint8_t *buff, size_t size, size_t max_dump);
#define print_buffer(level, dir, buff, size)                                                                                                         \
	if (0)                                                                                                                                           \
		(void)_kb__print_buffer;                                                                                                                     \
	DEBUG_START(level);                                                                                                                              \
	debug_print_prefix(debug_output, debug_buffer_remain, __FILE__, __LINE__);                                                                       \
	debug_output_move(_kb__print_buffer(debug_output, debug_buffer_remain, dir, buff, size, PRINT_BUFF_MAX));                                        \
	DEBUG_END()

#define debug_print_location(level, file, line, fmt, ...)                                                                                            \
	if (0)                                                                                                                                           \
		(void)0;                                                                                                                                     \
	DEBUG_START(level);                                                                                                                              \
	debug_print_prefix(debug_output, debug_buffer_remain, file, line);                                                                               \
	debug_printf(fmt __VA_OPT__(, ) __VA_ARGS__);                                                                                                    \
	DEBUG_END()

#define debug_print_win32(fmt, ...)                                                                                                                  \
	if (0)                                                                                                                                           \
		(void)0;                                                                                                                                     \
	DEBUG_START(KBURN_LOG_ERROR);                                                                                                                    \
	debug_print_prefix(debug_output, debug_buffer_remain, __FILE__, __LINE__);                                                                       \
	debug_printf(fmt ": " __VA_OPT__(, ) __VA_ARGS__);                                                                                               \
	debug_output_move(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0, (LPSTR)debug_output,        \
									debug_buffer_remain, NULL));                                                                                     \
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
		debug_printf("[%.*s] ", basename_to_ext_length(__FILE__), cbasename(__FILE__));                                                              \
	debug_printf(COLOR_FMT("%s") "(", COLOR_ARG(GREY, __func__));                                                                                    \
	debug_printf(__VaridicMacro_Opt(__clang_happy_fmt_string(""), __VA_ARGS__) __VaridicMacro_Shift(__VA_ARGS__));                                   \
	debug_printf(")");                                                                                                                               \
	DEBUG_END()
