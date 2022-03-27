#include "debug-print.h"
#include <stdarg.h>
#include <stdlib.h>

void m_assert_print_abort(const char *ncondition_str, const char *file, int line, const char *message, ...)
{
	debug_print_location(file, line, RED("Assertion Failed:") " %s", ncondition_str);

	va_list argptr;
	debug_print_head_location(file, line, "\t");

	va_start(argptr, message);
	vfprintf(stderr, message, argptr);
	fprintf(stderr, "\n");
	va_end(argptr);

	abort();
}
