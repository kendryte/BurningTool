#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

char *sprintf_alloc(const char *fmt, ...);
char *vsprintf_alloc(const char *fmt, va_list args);

static inline const char *OPTSTR(const char *str, const char *opt) { return str ? str : opt; }
static inline const char *NULLSTR(const char *str) { return OPTSTR(str, "<NULLSTR>"); }

static inline bool prefix(const char *pre, const char *str) { return strncmp(pre, str, strlen(pre)) == 0; }
