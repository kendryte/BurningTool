#include "base.h"
#include "basic/errors.h"
#include "components/device-link-list.h"
#include "device.h"
#include "lifecycle.h"
#include "private-types.h"

static void on_event(void *ctx, ser_dev_evt_t evt, const ser_dev_t *dev) {
	KBCTX scope = ctx;
	switch (evt) {
	case SER_DEV_EVT_ADDED:
		debug_print(KBURN_LOG_INFO, "[monitor] connect: %s", dev->path);
		on_serial_device_attach(scope, dev->path);
		break;
	case SER_DEV_EVT_REMOVED:
		debug_print(KBURN_LOG_INFO, "[monitor] remove : %s", dev->path);
		kburnDeviceNode *device = get_device_by_serial_port_path(scope, dev->path);
		if (device)
			destroy_serial_port(device->disposable_list, device);

		break;
	}
}

static void first_init_list(void *UNUSED(ctx), KBCTX scope, const bool *const quit) {
	debug_print(KBURN_LOG_INFO, "[init] init_list()");
	ser_dev_list_t *lst;

	lst = ser_dev_list_get();
	if (!lst) {
		debug_print(KBURN_LOG_ERROR, "serial port list get failed: %s", sererr_last());
		return;
	}

	ser_dev_list_t *item;

	ser_dev_list_foreach(item, lst) {
		if (*quit)
			break;

		if (prefix("/dev/ttyS", item->dev.path)) {
			continue;
		}
		debug_print(KBURN_LOG_INFO, "[init]   * %s", item->dev.path);
		on_serial_device_attach(scope, item->dev.path);
	}

	ser_dev_list_destroy(lst);
}

void serial_monitor_destroy(KBCTX scope) {
	debug_trace_function();
	if (!scope->serial->monitor_prepared)
		return;
	if (scope->serial->monitor_instance) {
		ser_dev_monitor_stop(scope->serial->monitor_instance);
		scope->serial->monitor_instance = NULL;
	}

	scope->serial->monitor_prepared = false;
}

kburn_err_t serial_monitor_prepare(KBCTX scope) {
	debug_trace_function();
	if (scope->serial->monitor_prepared)
		return KBurnNoErr;
	scope->serial->monitor_prepared = true;

	if (scope->serial->monitor_instance) {
		debug_print(KBURN_LOG_DEBUG, "\talready inited.");
		return KBurnNoErr;
	}

	thread_create("serial init list", first_init_list, NULL, scope, &scope->serial->init_list_thread);

	return KBurnNoErr;
}

void serial_monitor_pause(KBCTX scope) {
	debug_trace_function("[instance=%p]", (void *)scope->serial->monitor_instance);
	if (scope->serial->monitor_instance) {
		ser_dev_monitor_stop(scope->serial->monitor_instance);
		scope->serial->monitor_instance = NULL;
	}
}

kburn_err_t serial_monitor_resume(KBCTX scope) {
	debug_trace_function("[instance=%p]", (void *)scope->serial->monitor_instance);
	if (scope->serial->monitor_instance == NULL)
		scope->serial->monitor_instance = ser_dev_monitor_init(on_event, scope);

	if (scope->serial->monitor_instance == NULL) {
		debug_print(KBURN_LOG_ERROR, "ser_dev_monitor_init fail");
		return make_error_code(KBURN_ERROR_KIND_COMMON, KBurnSerialMonitorFailStart);
	}

	return KBurnNoErr;
}
