#pragma once

#include <stdbool.h>
#include "types.h"

/**
 * 库上下文，通过kburnCreate创建，所有函数都需要传此参数。主要用于跟踪内存资源
 */
typedef struct kburnContext
{
	on_device_remove disconnect_callback;
	void *disconnect_callback_ctx;

	struct serial_subsystem_context *const serial;
	struct usb_subsystem_context *const usb;
	struct disposable_list *const disposables;
	struct disposable_list *const threads;
	struct port_link_list *const openDeviceList;
	struct waiting_list *const waittingDevice;
	bool monitor_inited;
} kburnContext;

typedef kburnContext *KBCTX;
