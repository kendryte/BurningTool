#pragma once

#include "base.h"
#include "debug.h"
#include <stddef.h>

extern kburnDebugColors g_debug_colors;
#define RED g_debug_colors.red.prefix, g_debug_colors.red.postfix
#define GREEN g_debug_colors.green.prefix, g_debug_colors.green.postfix
#define YELLOW g_debug_colors.yellow.prefix, g_debug_colors.yellow.postfix
#define GREY g_debug_colors.grey.prefix, g_debug_colors.grey.postfix
size_t debug_format_color(const char *prefix, const char *postfix, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#define COLOR_FMT(const_str) "%s" const_str "%s"
static inline const char *COLOR_START(const char *prefix, const char *UNUSED(postfix)) { return prefix; }
static inline const char *COLOR_END(const char *UNUSED(prefix), const char *postfix) { return postfix; }

#define COLOR_ARG(COLOR, ...)                                                                                                                        \
	COLOR_START(COLOR)                                                                                                                               \
	__VA_OPT__(, )                                                                                                                                   \
	__VA_ARGS__, COLOR_END(COLOR)
