#include "global.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void global_resource_register(KBCTX scope, dispose_function callback, void *userData)
{
	dispose_add(scope->disposables, disposable(callback, userData));
}

void global_resource_unregister(KBCTX scope, dispose_function callback, void *userData)
{
	dispose_delete(scope->disposables, disposable(callback, userData));
}

#if WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void do_sleep(int ms)
{
#if WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

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
