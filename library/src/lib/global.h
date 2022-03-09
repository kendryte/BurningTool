#pragma once

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "canaan-burn.h"
#include "disposable.h"

void global_resource_register(dispose_function callback, void *userData);
void global_resource_unregister(dispose_function callback, void *userData);

#define HYPERLINK "\x1B]8;;%s\a%s\x1B]8;;\a" // url, text

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#define BASENAME(name) (strrchr(name, PATH_SEPARATOR) ? strrchr(name, PATH_SEPARATOR) + 1 : name)
#define gt_zero(x) (x > 0 ? x : 0)

#ifndef NDEBUG
#if TERMINAL_SUPPORT_HYPERLINK == 1
#define S(x) #x
#define S_(x) S(x)
#define debug_print(fmt, ...)                                                      \
	fprintf(stderr, "\x1B[2m[\x1B]8;;%s:%d\a%s:%d\x1B]8;;\a%*s]\x1B[0m " fmt "\n", \
			__FILE__, __LINE__,                                                    \
			BASENAME(__FILE__), __LINE__,                                          \
			(int)gt_zero(24 - strlen(BASENAME(__FILE__)) - strlen(S_(__LINE__))),  \
			"" __VA_OPT__(, ) __VA_ARGS__)
#else
#define RELATIVE_PATH (__FILE__ + strlen(PROJECT_ROOT))
#define debug_print(fmt, ...)                                                                                      \
	fprintf(stderr, "\x1B[2m[.%s:%-*d]\x1B[0m " fmt "\n", RELATIVE_PATH, gt_zero(43 - (int)strlen(RELATIVE_PATH)), \
			__LINE__ __VA_OPT__(, ) __VA_ARGS__)
#endif
#else

#define debug_print(fmt, ...)
#endif

static inline bool
prefix(const char *pre, const char *str)
{
	return strncmp(pre, str, strlen(pre)) == 0;
}

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif

static inline void lock(volatile int *lock)
{
	while (__sync_lock_test_and_set(lock, 1))
	{
	}
}

static inline void unlock(volatile int *lock)
{
	__sync_synchronize();
	*lock = 0;
}
