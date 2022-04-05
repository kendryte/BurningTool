#include "print.h"
#include "basic/string.h"
#include "context.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

size_t __print_buffer(char *output, size_t output_length, const char *dir, const uint8_t *buff, size_t size, size_t max_dump) {
	size_t ret = 0;
	ret += m_snprintf(output, output_length, "\n\t%s: \x1B[2m", dir);
	size_t cnt = size > max_dump ? max_dump : size;
	for (size_t i = 0; i < cnt; i++) {
		ret += m_snprintf(output + ret, output_length - ret, "0x%02X ", buff[i]);
	}
	ret += m_snprintf(output + ret, output_length - ret, "| %zu bytes\x1B[0m", size);
	return ret;
}
