#pragma once

#include <stdbool.h>

#define has_kb_mutex 1
#ifndef NDEBUG
#include <stdbool.h>
typedef struct kb_mutex *kb_mutex_t;
#else
#include <pthread.h>
typedef pthread_mutex_t *kb_mutex_t;
#endif
#include "canaan-burn/exported/types.h"

/**
 * 库上下文，通过kburnCreate创建，所有函数都需要传此参数。主要用于跟踪内存资源
 */
typedef struct kburnContext {
	on_device_remove_t on_disconnect;

	struct serial_subsystem_context *const serial;
	struct usb_subsystem_context *const usb;
	struct disposable_list *const disposables;
	struct disposable_list *const threads;
	struct port_link_list *const openDeviceList;
	struct waiting_list *const waittingDevice;
	struct event_queue_thread *user_event;

	struct dynamic_array *list1;
	struct dynamic_array *list2;

	bool monitor_inited;
} kburnContext;

typedef kburnContext *KBCTX;
