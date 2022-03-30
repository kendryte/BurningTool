#include "global.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef NDEBUG
__attribute__((access(read_only, 2, 3))) void __print_buffer(const char *dir, const uint8_t *buff, size_t size, size_t max_dump)
{
	fprintf(stderr, "\n\t%s: \x1B[2m", dir);
	size_t cnt = size > max_dump ? max_dump : size;
	for (size_t i = 0; i < cnt; i++)
	{
		fprintf(stderr, "0x%02X ", buff[i]);
	}
	fprintf(stderr, "| %ld bytes\x1B[0m\n", size);
}
#endif

char *sprintf_alloc(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	size_t size = vsnprintf(NULL, 0, fmt, args);
	char *ret = calloc(sizeof(char), size + 2);
	if (ret == NULL)
		return ret;
	vsnprintf(ret, size + 2, fmt, args);

	va_end(args);
	return ret;
}

char *vsprintf_alloc(const char *fmt, va_list args)
{
	va_list argsc;
	va_copy(argsc, args);
	size_t size = vsnprintf(NULL, 0, fmt, argsc);
	char *ret = calloc(sizeof(char), size + 100);
	if (ret == NULL)
		return ret;

	vsnprintf(ret, size + 100, fmt, args);

	return ret;
}
