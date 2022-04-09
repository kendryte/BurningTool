#include "path.h"
#include "basic/string.h"
#include <stdio.h>

const char *cbasename(const char *p) {
	char *a = strrchr(p, '/');
	char *b = strrchr(p, '\\');
	if (a > b) {
		return a + 1;
	} else {
		return b + 1;
	}
}

#ifndef DISABLE_TERM_HYPERLINK
#define FILE_LINE_FORMAT "\x1B]8;;%s:%d\a%s:%d\x1B]8;;\a"
#define FILE_LINE_VALUE(file_path, file_line) file_path, file_line, cbasename(file_path), file_line
#define PREFIX_SIZE 24
#else
#define FILE_LINE_FORMAT "%s:%d"
#define FILE_LINE_VALUE(file_path, file_line) relative_path(file_path), file_line
#define PREFIX_SIZE 42
#endif

const char *relative_path(const char *file_path) {
	return &file_path[strlen(PROJECT_ROOT) + 1];
}

int basename_to_ext_length(const char *name) {
	name = cbasename(name);
	char *found = strrchr(name, '.');
	if (found == NULL) {
		return strlen(name);
	}

	return found - name;
}

size_t debug_empty_prefix(char *output, size_t output_size) {
	return m_snprintf(output, output_size, "%*s", PREFIX_SIZE, "");
}

size_t _debug_format_path(char *output, size_t output_size, const char *file, const int line) {
	return m_snprintf(output, output_size, FILE_LINE_FORMAT, FILE_LINE_VALUE(file, line));
}

size_t _debug_format_prefix(char *output, size_t output_size, const char *file, const int line) {
	size_t size = 1;
	*output = '[';

	size += _debug_format_path(output + size, output_size - size, file, line);

	if (size < PREFIX_SIZE) {
		size += m_snprintf(output + size, output_size - size, "%*s", PREFIX_SIZE - (int)size, "");
	}

	output[size] = ']';
	output[size + 1] = ' ';
	output[size + 2] = '\0';

	return size + 2;
}

#ifndef NDEBUG

size_t _debug_format_bundle_title(char *output, size_t output_size, debug_bundle e) {
	size_t size = 0;
	if (e.func) {
		size += m_snprintf(output, output_size, "%s::", e.func);
	}

	size += m_snprintf(output + size, output_size - size, "%s", OPTSTR(e.title, "<anonymouse>"));

	return size;
}
#endif
