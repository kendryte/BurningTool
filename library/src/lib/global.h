#pragma once

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "canaan-burn.h"
#include "./device.h"
#include "./disposable.h"

#define MAX_WAITTING_DEVICES 32
/**
 * 库上下文，通过kburnCreate创建，所有函数都需要传此参数。主要用于跟踪内存资源
 */
typedef struct kburnContext
{
	struct serial_subsystem_context *const serial;
	struct usb_subsystem_context *const usb;
	struct disposable_registry *const disposables;
	struct port_link_list *const openDeviceList;
	struct
	{
		volatile int lock;
		kburnSerialDeviceNode *list[MAX_WAITTING_DEVICES + 1];
	} waittingDevice;
	bool monitor_inited;
} kburnContext;

typedef kburnContext *KBCTX;

void global_resource_register(KBCTX scope, dispose_function callback, void *userData);
void global_resource_unregister(KBCTX scope, dispose_function callback, void *userData);

#define HYPERLINK "\x1B]8;;%s\a%s\x1B]8;;\a" // url, text

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#define BASENAME(name) (strrchr(name, PATH_SEPARATOR) ? strrchr(name, PATH_SEPARATOR) + 1 : name)
#define gt_zero(x) (x > 0 ? x : 0)

#ifndef NDEBUG

#ifndef DISABLE_TERM_HYPERLINK
#define S(x) #x
#define S_(x) S(x)
#define debug_print(fmt, ...)                                                      \
	fprintf(stderr, "\x1B[2m[\x1B]8;;%s:%d\a%s:%d\x1B]8;;\a%*s]\x1B[0m " fmt "\n", \
			__FILE__, __LINE__,                                                    \
			BASENAME(__FILE__), __LINE__,                                          \
			(int)gt_zero(24 - strlen(BASENAME(__FILE__)) - strlen(S_(__LINE__))),  \
			"" __VA_OPT__(, ) __VA_ARGS__)

#define debug_print_head(title)                                                \
	fprintf(stderr, "\x1B[2m[\x1B]8;;%s:%d\a%s:%d\x1B]8;;\a%*s]\x1B[0m" title, \
			__FILE__, __LINE__,                                                \
			BASENAME(__FILE__), __LINE__,                                      \
			(int)gt_zero(24 - strlen(BASENAME(__FILE__)) - strlen(S_(__LINE__))))

#else // DISABLE_TERM_HYPERLINK

#define RELATIVE_PATH (__FILE__ + strlen(PROJECT_ROOT))
#define debug_print(fmt, ...)                                                                                      \
	fprintf(stderr, "\x1B[2m[.%s:%-*d]\x1B[0m " fmt "\n", RELATIVE_PATH, gt_zero(43 - (int)strlen(RELATIVE_PATH)), \
			__LINE__ __VA_OPT__(, ) __VA_ARGS__)
#define debug_print_head(title) \
	fprintf(stderr, "\x1B[2m[.%s:%-*d]\x1B[0m" title, RELATIVE_PATH, gt_zero(43 - (int)strlen(RELATIVE_PATH)), __LINE__)

#endif // DISABLE_TERM_HYPERLINK

#else // NDEBUG

#define debug_print(fmt, ...)
#define debug_print_head(title)

#endif // NDEBUG

static inline const char *NULLSTR(const char *str)
{
	return str ? str : "<NULLSTR>";
}

#define TODO debug_print("\x1B[38;5;11mTODO in %s()\x1B[0m", __func__)

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

#ifndef NDEBUG
#define lock(x) __extension__({                         \
	int lock_time = 0;                                  \
	while (__sync_lock_test_and_set((x), 1))            \
	{                                                   \
		do_sleep(100);                                  \
		lock_time++;                                    \
		if (lock_time == 100)                           \
		{                                               \
			debug_print("lock [" #x "] take too long"); \
		}                                               \
	}                                                   \
})

#define unlock(l) __extension__({ \
	__sync_synchronize();         \
	(*l) = 0;                     \
})
#else
static inline void __lock(volatile int *lock)
{
	while (__sync_lock_test_and_set(lock, 1))
	{
		do_sleep(100);
	}
}

static inline void __unlock(volatile int *lock)
{
	__sync_synchronize();
	*lock = 0;
}
#endif

#ifndef NDEBUG
#define print_buffer(dir, buff, size) \
	debug_print_head();               \
	__print_buffer(dir, buff, size, 30)

void __print_buffer(const char *dir, const uint8_t *buff, size_t size, size_t max_dump);
#else
#define print_buffer(...)
#endif

void do_sleep(int ms);

kburnSerialDeviceInfo driver_get_devinfo(const char *path);
void driver_get_devinfo_free(kburnSerialDeviceInfo);

#define __KBALLOC_2(type, cnt) (type *)__extension__({ \
	void *v##__LINE__ = calloc(cnt, sizeof(type));     \
	if (v##__LINE__ == NULL)                           \
		return KBurnNoMemory;                          \
	v##__LINE__;                                       \
})
#define __KBALLOC_1(type) __KBALLOC_2(type, 1)
#define __KBALLOC_X(A0, A1, A2, FN, ...) FN

#define KBALLOC(...) __KBALLOC_X(, ##__VA_ARGS__,          \
								 __KBALLOC_2(__VA_ARGS__), \
								 __KBALLOC_1(__VA_ARGS__))

#ifdef __LITTLE_ENDIAN__
#include <byteswap.h>
#define CHIP_ENDIAN32(x) bswap_32(x)
#else
#define CHIP_ENDIAN32(x) x
#endif

typedef void (*thread_function)(KBCTX scope, const bool *const quit);
typedef struct thread_passing_object *kbthread;
kburn_err_t thread_create(const char *debug_title, thread_function start_routine, KBCTX scope, kbthread *out_thread);
void thread_tell_quit(kbthread thread);
void thread_wait_quit(kbthread thread);

typedef struct queue_info *queue_t;
kburn_err_t queue_create(queue_t *queue);
void queue_destroy(queue_t queue);
kburn_err_t queue_push(queue_t queue, void *data, bool free_when_destroy);
void *queue_shift(queue_t queue);

char *sprintf_alloc(const char *fmt, ...);
