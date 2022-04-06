#pragma once

#include "low.h"
#include <stddef.h>

const char *relative_path(const char *file_path);
const char *cbasename(const char *p);
int basename_to_ext_length(const char *name);
size_t _debug_format_prefix(char *output, size_t output_size, const char *file, int line);
size_t _debug_format_path(char *output, size_t output_size, const char *file, const int line);

#ifndef NDEBUG
typedef struct debug_bundle {
	char _null;
	const char *title;
	const char *func;
	const char *file;

	char buff1[256];
	char buff2[256];

	int line;
} debug_bundle;

static inline debug_bundle __DEBUG_SAVE(const char *title, const char *file, int line, const char *func) {
	return (debug_bundle){.title = title, ._null = '\0', .func = func, .file = file, .line = line};
}
#define DEBUG_SAVE(title) __DEBUG_SAVE(title, __FILE__, __LINE__, __func__)

#define struct_debug_bundle struct debug_bundle

size_t _debug_format_bundle_title(char *output, size_t output_size, debug_bundle e);

#define debug_print_prefix(output, output_size, file_name, file_line)                                                                                \
	debug_output_move(_debug_format_prefix(output, output_size, file_name, file_line))

#define DEBUG_OBJ_TITLE(dbg) (_debug_format_bundle_title(dbg.buff1, 512, dbg), dbg.buff1)

#define DEBUG_OBJ_PATH(dbg) (_debug_format_path(dbg.buff2 + 3, 512 - 3, dbg.file, dbg.line), dbg.buff2)

#define debug_print_dbg(level, dbg, fmt, ...) debug_print_location(level, dbg.file, dbg.line, fmt __VA_OPT__(, ) __VA_ARGS__)

#else
#define struct_debug_bundle const char *
static inline const char *DEBUG_SAVE(const char *title) { return title; }

#define debug_print_prefix(output, output_size, file_name, file_line)
#define DEBUG_OBJ_TITLE(dbg) (const char *)dbg
#define DEBUG_OBJ_PATH(dbg) ""

#define debug_print_dbg(level, dbg, fmt, ...) debug_print_location(level, dbg.file, dbg.line, fmt __VA_OPT__(, ) __VA_ARGS__)

#endif
