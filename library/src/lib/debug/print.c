#include "print.h"
#include "canaan-burn/canaan-burn.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "basic/string.h"

void _debug_print(const char *file, int line, const char *fmt, ...)
{
	debug_output_move(debug_format_prefix(debug_output, debug_buffer_remain, file, line));

	va_list va;
	va_start(va, fmt);
	__debug_vprintf(fmt, va);
	va_end(va);
}

static inline size_t __print_buffer(char *output, size_t output_length, const char *dir, const uint8_t *buff, size_t size, size_t max_dump)
{
	size_t ret = 0;
	ret += m_snprintf(output, output_length, "\n\t%s: \x1B[2m", dir);
	size_t cnt = size > max_dump ? max_dump : size;
	for (size_t i = 0; i < cnt; i++)
	{
		ret += m_snprintf(output + ret, output_length - ret, "0x%02X ", buff[i]);
	}
	ret += m_snprintf(output + ret, output_length - ret, "| %zu bytes\x1B[0m", size);
	return ret;
}

void _print_buffer(const char *file, int line, const char *dir, const uint8_t *buff, size_t size, size_t max_dump)
{
	debug_output_move(debug_format_prefix(debug_output, debug_buffer_remain, file, line));
	debug_output_move(__print_buffer(debug_output, debug_buffer_remain, dir, buff, size, max_dump));
}

void _debug_trace_function(const char *file, int line, const char *func, const char *fmt, ...)
{
	debug_output_move(debug_format_prefix(debug_output, debug_buffer_remain, file, line));
	debug_printf("[%.*s] " COLOR_FMT("%s") "(", basename_to_ext_length(file), basename(file), COLOR_ARG(GREY, func));

	if (fmt != NULL)
	{
		va_list va;
		va_start(va, fmt);
		__debug_vprintf(fmt, va);
		va_end(va);
	}

	debug_puts(")");
}
