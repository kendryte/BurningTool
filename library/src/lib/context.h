#pragma once

#include "canaan-burn/exported/types.h"
#include <stdbool.h>
#include <string.h>

#define CONTEXT_MEMORY_SIGNATURE "kburnCtx"

/**
 * 库上下文，通过kburnCreate创建，所有函数都需要传此参数。主要用于跟踪内存资源
 */
typedef struct kburnContext {
	char signature[sizeof(CONTEXT_MEMORY_SIGNATURE)];
	on_device_remove_t on_disconnect;
	on_device_list_change_t on_list_change;

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

static inline bool validateContext(KBCTX ptr) {
	return memcmp(ptr->signature, CONTEXT_MEMORY_SIGNATURE, sizeof(CONTEXT_MEMORY_SIGNATURE)) == 0;
}
