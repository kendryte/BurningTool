#pragma once

#include "base.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static inline const char *__clang_happy_fmt_string(const char *_) { return _; }
static inline size_t __smart_string_length(const char *s, size_t otherwise) { return (s == NULL) ? otherwise : strlen(s); }

#define m_snprintf(output, size, fmt, ...)                                                                                                           \
	__extension__({                                                                                                                                  \
		if (0)                                                                                                                                       \
			(void)0;                                                                                                                                 \
		size_t b = snprintf(output, size, fmt __VA_OPT__(, ) __VA_ARGS__);                                                                           \
		__smart_string_length(output, b);                                                                                                            \
	})
#define m_vsnprintf(output, size, fmt, va)                                                                                                           \
	__extension__({                                                                                                                                  \
		size_t b = vsnprintf(output, size, fmt, va);                                                                                                 \
		__smart_string_length(output, b);                                                                                                            \
	})

#define buffer_make(name, size)                                                                                                                      \
	const size_t CONCAT(name, _length) = size;                                                                                                       \
	char CONCAT(name, _base)[size];                                                                                                                  \
	char *name = CONCAT(name, _base);
#define buffer_remain(name) (size_t)(CONCAT(name, _length) - (name - CONCAT(name, _base)))
#define buffer_move(name) name +=

char *sprintf_alloc(const char *fmt, ...);
char *vsprintf_alloc(const char *fmt, va_list args);

static inline const char *OPTSTR(const char *str, const char *opt) { return str ? str : opt; }
static inline const char *NULLSTR(const char *str) { return OPTSTR(str, "<NULLSTR>"); }

static inline bool prefix(const char *pre, const char *str) { return strncmp(pre, str, strlen(pre)) == 0; }
