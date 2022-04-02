#include "basic/errors.h"
#include "components/call-user-handler.h"
#include "components/device-link-list.h"
#include "serial.h"

kburn_err_t on_serial_device_attach(KBCTX scope, const char *path) {
	DeferEnabled;

	debug_print(KBURN_LOG_TRACE, COLOR_FMT("on_serial_device_attach") "(%s)", COLOR_ARG(GREEN), path);

	kburnDeviceNode *node = NULL;

	IfErrorReturn(create_empty_device_instance(scope, &node));
	DeferDispose(scope->disposables, node, destroy_device);
	debug_print(KBURN_LOG_INFO, "new device created: %p", (void *)node);

	DeferUserCallback(scope->serial->handler_callback, node, true);

	IfErrorReturn(serial_port_init(node->serial, path));
	dispose_list_add(node->disposable_list, toDisposable(destroy_serial_port, node));

	if (scope->serial->verify_callback != NULL) {
		if (!scope->serial->verify_callback(node, scope->serial->verify_callback_ctx)) {
			set_error(node, KBURN_ERROR_KIND_COMMON, KBurnUserCancel, "operation canceled by verify callback");
			return make_error_code(KBURN_ERROR_KIND_COMMON, KBurnUserCancel);
		}
	}
	debug_print(KBURN_LOG_INFO, "user verify pass");

	if (!node->serial->isOpen) {
		if (!serial_low_open(node->serial)) {
			debug_print(KBURN_LOG_ERROR, "open failed");
			return node->error->code;
		}
	}

	if (!node->serial->isConfirm) {
		if (!confirm_port_is_ready(node->serial)) {
			debug_print(KBURN_LOG_ERROR, "confirm failed");
			return node->error->code;
		}
	}

	add_to_device_list(node);
	node->disconnect_should_call = true;

	DeferAbort;
	return KBurnNoErr;
}
