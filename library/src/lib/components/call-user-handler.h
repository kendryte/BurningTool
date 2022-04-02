#pragma once

#include "types.h"
#include "base.h"
#include "context.h"
#include "basic/disposable.h"

struct user_handler_wrap_data
{
	on_device_handle handler;
	void *context;
	kburnDeviceNode *device;
};

void user_handler_wrap(struct user_handler_wrap_data *data);

#define DeferUserCallback(callback, device, always) __extension__({                                      \
	if (callback == NULL)                                                                                \
	{                                                                                                    \
		debug_print(KBURN_LOG_ERROR, "No handler registed for " #callback);                              \
	}                                                                                                    \
	else                                                                                                 \
	{                                                                                                    \
		struct user_handler_wrap_data *_user_handler_wrap_data = MyAlloc(struct user_handler_wrap_data); \
		DeferFreeAlways(_user_handler_wrap_data);                                                        \
		*_user_handler_wrap_data = (struct user_handler_wrap_data){                                      \
			callback,                                                                                    \
			callback##_ctx,                                                                              \
			device,                                                                                      \
		};                                                                                               \
		if (always)                                                                                      \
			DeferCallAlways(user_handler_wrap, _user_handler_wrap_data);                                 \
		else                                                                                             \
			DeferCall(user_handler_wrap, _user_handler_wrap_data);                                       \
	}                                                                                                    \
})
#define CALL_USER(handler, device) __extension__({                                           \
	struct user_handler_wrap_data _user_handler_wrap_data = (struct user_handler_wrap_data){ \
		callback,                                                                            \
		callback##_ctx,                                                                      \
		device,                                                                              \
	};                                                                                       \
	user_handler_wrap(&_user_handler_wrap_data)                                              \
})

kburn_err_t global_init_user_handle_thread(KBCTX scope);
