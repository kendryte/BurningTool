#include "debug-print.h"
#include <stdarg.h>
#include <stdlib.h>

void m_assert_print_abort(const char *ncondition_str, const char *file, int line, const char *message, ...)
{
	debug_print_location(file, line, YELLO("Assertion Failed:") " %s", ncondition_str);

	va_list argptr;
	debug_print_head_location(file, line, "\t");

	va_start(argptr, message);
	vfprintf(stderr, message, argptr);
	fprintf(stderr, "\n");
	va_end(argptr);

	abort();
}

void debug_print_bundle(const char *fmt, struct debug_bundle debug, ...)
{
	char buff[1024];

	va_list argptr;
	va_start(argptr, debug);
	vsnprintf(buff, 1024, fmt, argptr);
	va_end(argptr);

	char *f = strstr(buff, "$b");
	if (f == NULL)
		m_abort("format string \"%s\" do not have $b");
	*f = '\0';
	f += 2;

	if (debug.func && debug.title)
	{
		debug_print_location(debug.file, debug.line, "%s%s::%s%s", buff, debug.func, debug.title, f);
	}
	else if (debug.func)
	{
		debug_print_location(debug.file, debug.line, "%s%s::<anonymouse>%s", buff, debug.func, f);
	}
	else if (debug.title)
	{
		debug_print_location(debug.file, debug.line, "%s%s%s", buff, debug.title, f);
	}
	else
	{
		debug_print_location(debug.file, debug.line, "%s<anonymouse>%s", buff, f);
	}
}
