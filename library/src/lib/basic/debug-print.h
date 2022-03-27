#pragma once
#include <stdio.h>
#include "base.h"

#define HYPERLINK "\x1B]8;;%s\a%s\x1B]8;;\a" // url, text

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#define BASENAME(name) (strrchr(name, PATH_SEPARATOR) ? strrchr(name, PATH_SEPARATOR) + 1 : name)
#define gt_zero(val) __extension__({ int x=val; x > 0 ? x : 0; })

#ifndef NDEBUG

#ifndef DISABLE_TERM_HYPERLINK
#define FILE_LINE_FORMAT "\x1B]8;;%s:%d\a%s:%d\x1B]8;;\a"
#define FILE_LINE_VALUE(file_path, file_line) file_path, file_line, BASENAME(file_path), file_line

#define debug_print_location(file_path, file_line, fmt, ...)                               \
	fprintf(stderr, "\x1B[2m[" FILE_LINE_FORMAT "%*s]\x1B[0m " fmt "\n",                   \
			FILE_LINE_VALUE(file_path, file_line),                                         \
			(int)gt_zero(24 - strlen(BASENAME(file_path)) - strlen(STRINGIZE(file_line))), \
						 "" __VA_OPT__(, ) __VA_ARGS__)

#define debug_print_head_location(file_path, file_line, title)       \
	fprintf(stderr, "\x1B[2m[" FILE_LINE_FORMAT "%*s]\x1B[0m" title, \
			FILE_LINE_VALUE(file_path, file_line),                   \
			(int)gt_zero(24 - strlen(BASENAME(file_path)) - strlen(STRINGIZE(file_line))))

#else // DISABLE_TERM_HYPERLINK
#include <string.h>
#define FILE_LINE_FORMAT ".%s:%d"
#define FILE_LINE_VALUE(file_path, file_line) RELATIVE_PATH(file_path), file_line

#ifdef __FILENAME__
#define RELATIVE_PATH(file_path) file_path
#else
#define RELATIVE_PATH(file_path) (&file_path[strlen(PROJECT_ROOT)])
#endif
#define debug_print_location(file_path, file_line, fmt, ...) \
	fprintf(stderr, "\x1B[2m[" FILE_LINE_FORMAT "%*s]\x1B[0m " fmt "\n", FILE_LINE_VALUE(file_path, file_line), (int)gt_zero(42 - strlen(RELATIVE_PATH(file_path)) - strlen(STRINGIZE(file_line))), "" __VA_OPT__(, ) __VA_ARGS__)
#define debug_print_head_location(file_path, file_line, title) \
	fprintf(stderr, "\x1B[2m[" FILE_LINE_FORMAT "%*s]\x1B[0m%s", FILE_LINE_VALUE(file_path, file_line), (int)gt_zero(42 - strlen(RELATIVE_PATH(file_path)) - strlen(STRINGIZE(file_line))), "", title)

#endif // DISABLE_TERM_HYPERLINK

#else // NDEBUG

#define FILE_LINE_FORMAT ".%s:%d"
#define FILE_LINE_VALUE(file_path, file_line) file_path, file_line

inline static void debug_print_location(const char *file, int UNUSED(line), const char *UNUSED(fmt), ...) { (void)file; }
inline static void debug_print_head_location(const char *UNUSED(file), int UNUSED(line), const char *UNUSED(t)) {}

#endif // NDEBUG

#define debug_print(fmt, ...) debug_print_location(__FILENAME__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define debug_print_head(title, ...) debug_print_head_location(__FILENAME__, __LINE__, title)

#define TODO debug_print("\x1B[38;5;11mTODO in %s()\x1B[0m", __func__)

#ifndef NDEBUG
#define print_buffer(dir, buff, size) \
	debug_print_head("");             \
	__print_buffer(dir, buff, size, 30)

void __print_buffer(const char *dir, const uint8_t *buff, size_t size, size_t max_dump);
#else
#define print_buffer(...)
#endif

#define RED(str) "\x1B[38;5;9m" str "\x1B[0m"
#define GREEN(str) "\x1B[38;5;10m" str "\x1B[0m"
#define YELLO(str) "\x1B[38;5;11m" str "\x1B[0m"
#define GREY(str) "\x1B[2m" str "\x1B[0m"

_Noreturn void m_assert_print_abort(const char *ncondition_str, const char *file, int line, const char *message, ...);
#define m_abort(fmt, ...) m_assert_print_abort("abort()", __FILENAME__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define m_assert(ncondition, fmt, ...) \
	if (!(ncondition))                 \
	m_assert_print_abort(#ncondition, __FILENAME__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)

#define m_assert0(value, exmsg) __extension__({                                         \
	bool va = (value) == 0;                                                             \
	m_assert(va, #value " == 0", "value of %s is %d (except 0) %s", #value, va, exmsg); \
})

#define m_assert_ptr(ptr, fmt, ...) __extension__({      \
	void *p = ptr;                                       \
	m_assert(p != NULL, fmt __VA_OPT__(, ) __VA_ARGS__); \
})

#define m_assert_err(value, exmsg) __extension__({                                                     \
	kburnErrorDesc e = kburnSplitErrorCode(value);                                                     \
	m_assert(e.kind == 0 && e.code == 0, "Error [kind=%d, code=%d]: %s", e.kind >> 32, e.code, exmsg); \
})
