#include "global.h"
#include "basic/string.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

char *sprintf_alloc(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);

	size_t size = m_vsnprintf(NULL, 0, fmt, args);
	char *ret = calloc(sizeof(char), size + 2);
	if (ret == NULL)
		return ret;
	m_vsnprintf(ret, size + 2, fmt, args);

	va_end(args);
	return ret;
}

char *vsprintf_alloc(const char *fmt, va_list args) {
	va_list argsc;
	va_copy(argsc, args);
	size_t size = m_vsnprintf(NULL, 0, fmt, argsc);
	char *ret = calloc(sizeof(char), size + 100);
	if (ret == NULL)
		return ret;

	m_vsnprintf(ret, size + 100, fmt, args);

	return ret;
}
