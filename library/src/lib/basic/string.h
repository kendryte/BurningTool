#pragma once

#include "base.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static inline char *__empty_string(const char *UNUSED(_)) { return NULL; }
static inline const char *__none_empty_string(const char *_) { return _; }

#define m_snprintf(output, size, fmt, ...)                                                                                                           \
	__extension__({                                                                                                                                  \
		snprintf(output, size, fmt __VA_OPT__(, ) __VA_ARGS__);                                                                                      \
		(output == NULL) ? 0 : strlen(__none_empty_string(output));                                                                                  \
	})
#define m_vsnprintf(output, size, fmt, va)                                                                                                           \
	__extension__({                                                                                                                                  \
		vsnprintf(output, size, fmt, va);                                                                                                            \
		(output == NULL) ? 0 : strlen(__none_empty_string(output));                                                                                  \
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
