#include "lifecycle-helper.h"
#include "device.h"

static void destroy_failed_device_in_thread(void *ctx, kburnDeviceNode *node) {
	KBCTX scope = ctx;
	destroy_device(scope->disposables, node);
}

void call_handler_wrapper(struct user_handler_wrap_data *data) {
	user_handler_wrap_async(data);
	if (!data->device->disconnect_should_call) { // init NOT complete
		struct user_handler_wrap_data *ndata = calloc(1, sizeof(struct user_handler_wrap_data));
		ndata->device = data->device;
		ndata->context = data->device->_scope;
		ndata->handler = destroy_failed_device_in_thread;
		user_handler_wrap_async(ndata);
	}
}
