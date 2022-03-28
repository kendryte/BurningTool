#pragma once

#include "base.h"
#include <stdio.h>
#include <string.h>
#include "canaan-burn/canaan-burn.h"
#include "device.h"
#include "basic/lock.h"
#include "basic/disposable.h"

/**
 * 库上下文，通过kburnCreate创建，所有函数都需要传此参数。主要用于跟踪内存资源
 */
typedef struct kburnContext
{
	struct serial_subsystem_context *const serial;
	struct usb_subsystem_context *const usb;
	disposable_list_t *const disposables;
	disposable_list_t *const threads;
	struct port_link_list *const openDeviceList;
	struct waiting_list *const waittingDevice;
	bool monitor_inited;
} kburnContext;

typedef kburnContext *KBCTX;

void global_resource_unregister(KBCTX scope, dispose_function callback, void *userData);

#include "basic/sleep.h"

typedef void (*thread_function)(KBCTX scope, const bool *const quit);
typedef struct thread_passing_object *kbthread;
kburn_err_t thread_create(const char *debug_title, thread_function start_routine, KBCTX scope, kbthread *out_thread);

typedef struct queue_info *queue_t;
kburn_err_t queue_create(queue_t *queue);
void queue_destroy(queue_t queue);
kburn_err_t queue_push(queue_t queue, void *data, bool free_when_destroy);
void *queue_shift(queue_t queue);

char *sprintf_alloc(const char *fmt, ...);

kburnSerialDeviceInfo driver_get_devinfo(const char *path);
void driver_get_devinfo_free(kburnSerialDeviceInfo);

static inline const char *NULLSTR(const char *str)
{
	return str ? str : "<NULLSTR>";
}

static inline bool
prefix(const char *pre, const char *str)
{
	return strncmp(pre, str, strlen(pre)) == 0;
}

#define FREE_WHEN_RETURN(var) var __attribute__((__cleanup__(free_pointer)))
#define DESTROY_WHEN_RETURN(var, cleanup_function) var __attribute__((__cleanup__(cleanup_function)))
#define IfErrorReturn(action) __extension__({ \
	kburn_err_t _err = action;                \
	if (_err != KBurnNoErr)                   \
		return _err;                          \
	_err;                                     \
})

#include "basic/alloc.h"
#include "basic/endian.h"
#include "basic/debug-print.h"
#include "basic/resource-tracker.h"
