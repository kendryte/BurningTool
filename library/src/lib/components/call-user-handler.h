#pragma once

#include "types.h"
#include "base.h"

struct user_handler_wrap_data
{
	on_device_handle handler;
	void *context;
	kburnDeviceNode *device;
};

inline static void user_handler_wrap(const struct user_handler_wrap_data *const data)
{
	data->handler(data->device, data->context);
}

#define DeferUserCallback(callback, device, always) __extension__({                                  \
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
})
#define CALL_USER(handler, device) __extension__({                                           \
	struct user_handler_wrap_data _user_handler_wrap_data = (struct user_handler_wrap_data){ \
		callback,                                                                            \
		callback##_ctx,                                                                      \
		device,                                                                              \
	};                                                                                       \
	user_handler_wrap(&_user_handler_wrap_data)                                              \
})
