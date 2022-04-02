#include "call-user-handler.h"
#include "queued-thread.h"

void user_callback_thread(KBCTX scope, void *_evt)
{
	(void)scope;
	const struct user_handler_wrap_data *data = _evt;

	data->handler(data->device, data->context);
}
kburn_err_t global_init_user_handle_thread(KBCTX scope)
{
	return event_thread_init(scope, "userCb", user_callback_thread, &scope->user_event);
}

void user_handler_wrap(struct user_handler_wrap_data *data)
{
	event_thread_queue(data->device->_scope->user_event, data);
}
