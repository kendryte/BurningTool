#include "basic/errors.h"
#include "basic/resource-tracker.h"
#include "components/call-user-handler.h"
#include "components/device-link-list.h"
#include "components/lifecycle-helper.h"
#include "device.h"
#include "driver.h"
#include "isp.h"
#include "low.h"
#include "private-types.h"

static bool confirm_port_is_ready(kburnSerialDeviceNode *node) {
	if (!kburnSerialIspGreeting(node)) {
		return false;
	}

	node->isConfirm = true;
	return true;
}

static inline void free_handle(void *handle) {
	if (handle)
		free(handle);
}

DECALRE_DISPOSE(destroy_serial_port, kburnDeviceNode) {
	kburnSerialDeviceNode *serial = context->serial;
	debug_trace_function("%p[%s]", (void *)serial, OPTSTR(serial->deviceInfo.path, "*invalid*"));

	if (!serial->init)
		return;

	lock(serial->mutex);

	serial_isp_delete(serial);

	if (serial->binding) {
		free(serial->binding);
		serial->binding = NULL;
	}

	if (serial->isOpen)
		serial_low_close(serial);

	if (serial->m_dev_handle != NULL)
		ser_destroy(serial->m_dev_handle);

	serial->init = false;

	unlock(serial->mutex);
	lock_deinit(&serial->mutex);
	device_instance_collect(context->_scope, context);
}
DECALRE_DISPOSE_END()

kburn_err_t serial_port_init(kburnSerialDeviceNode *serial, const char *path) {
	debug_trace_function("0x%p, %s", (void *)serial, path);

	m_assert(!serial->init, "serial port must not already inited");

	serial->deviceInfo = driver_get_devinfo(path);
	serial->mutex = CheckNull(lock_init());

	serial_isp_open(serial);

	serial->init = true;

	return KBurnNoErr;
}

kburn_err_t on_serial_device_attach(KBCTX scope, const char *path, bool need_verify) {
	DeferEnabled;

	debug_print(KBURN_LOG_TRACE, COLOR_FMT("on_serial_device_attach") "(%s, %d)", COLOR_ARG(GREEN), path, need_verify);

	kburnDeviceNode *node = NULL;

	IfErrorReturn(create_empty_device_instance(scope, &node));
	DeferDispose(scope->disposables, node, destroy_device);
	debug_print(KBURN_LOG_INFO, "new device created: %p", (void *)node);

	IfErrorReturn(serial_port_init(node->serial, path));
	dispose_list_add(node->disposable_list, toDisposable(destroy_serial_port, node));

	if (need_verify && (scope->serial->on_verify.handler != NULL)) {
		if (!CALL_HANDLE_SYNC(scope->serial->on_verify, node)) {
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

	CALL_HANDLE_ASYNC(scope->serial->on_handle, node);

	return KBurnNoErr;
}
