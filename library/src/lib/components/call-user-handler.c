#include "call-user-handler.h"
#include "queued-thread.h"

static void user_callback_thread(KBCTX UNUSED(scope), void *_evt) {
	struct user_handler_wrap_data *data = _evt;

	data->handler(data->context, data->device);
	free(data);
}
kburn_err_t global_init_user_handle_thread(KBCTX scope) {
	return event_thread_init(scope, "userCb", user_callback_thread, &scope->user_event);
}

void user_handler_wrap_async(struct user_handler_wrap_data *data) {
	event_thread_queue(data->device->_scope->user_event, data);
}
void user_handler_wrap_sync(struct user_handler_wrap_data *data) {
	user_callback_thread(NULL, data);
}

void _user_handler_wrap_async_new(on_device_handle handler, void *context, kburnDeviceNode *device) {
	struct user_handler_wrap_data *data = calloc(1, sizeof(struct user_handler_wrap_data));
	data->context = context;
	data->handler = handler;
	data->device = device;
	event_thread_queue(data->device->_scope->user_event, data);
}
