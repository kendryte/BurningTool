#pragma once

#include "base.h"
#include "context.h"
#include "basic/disposable.h"

struct user_handler_wrap_data {
	on_device_handle handler;
	void *context;
	kburnDeviceNode *device;
};

#define CALL_HANDLE_SYNC(bundle, ...) bundle.handler(bundle.context, __VA_ARGS__)
#define CALL_HANDLE_ASYNC(callback, device) _user_handler_wrap_async_new(callback.handler, callback.context, device)

void _user_handler_wrap_async_new(on_device_handle handler, void *context, kburnDeviceNode *device);
void user_handler_wrap_async(struct user_handler_wrap_data *data);
void user_handler_wrap_sync(struct user_handler_wrap_data *data);

#define DeferUserCallback(wrapper, callback, device)                                                         \
	__extension__({                                                                                          \
		if (0)                                                                                               \
			(void)user_handler_wrap_sync;                                                                    \
		if (callback.handler == NULL) {                                                                      \
			debug_print(KBURN_LOG_ERROR, "No handler registed for " #callback);                              \
		} else {                                                                                             \
			struct user_handler_wrap_data *_user_handler_wrap_data = MyAlloc(struct user_handler_wrap_data); \
			*_user_handler_wrap_data = (struct user_handler_wrap_data){                                      \
				callback.handler,                                                                            \
				callback.context,                                                                            \
				device,                                                                                      \
			};                                                                                               \
			DeferCallAlways(user_handler_wrap_sync, _user_handler_wrap_data);                                \
		}                                                                                                    \
	})

kburn_err_t global_init_user_handle_thread(KBCTX scope);
