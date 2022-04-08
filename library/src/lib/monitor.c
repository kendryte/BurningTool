#include "../usb/monitor.h"
#include "../serial/monitor.h"
#include "../serial/private-types.h"
#include "../serial/subsystem.h"
#include "../usb/private-types.h"
#include "../usb/subsystem.h"
#include "basic/resource-tracker.h"
#include "canaan-burn/canaan-burn.h"

static DECALRE_DISPOSE(_dispose, kburnContext) {
	if (context->monitor_inited) {
		usb_monitor_pause(context);
		serial_monitor_pause(context);

		context->monitor_inited = false;
	}
	if (context->serial->subsystem_inited) {
		serial_subsystem_init(context);
	}
	if (context->usb->subsystem_inited) {
		usb_subsystem_deinit(context);
	}
}
DECALRE_DISPOSE_END()

kburn_err_t kburnStartWaitingDevices(KBCTX scope) {
	debug_trace_function("already_init=%d", scope->monitor_inited);
	if (scope->monitor_inited) {
		return KBurnNoErr;
	}

	DeferEnabled;

	kburn_err_t r;

	if (!scope->usb->subsystem_inited) {
		usb_subsystem_init(scope);
		DeferCall(usb_subsystem_deinit, scope);
	}

	if (!scope->serial->subsystem_inited) {
		serial_subsystem_init(scope);
		DeferCall(serial_subsystem_deinit, scope);
	}

	r = serial_monitor_prepare(scope);
	if (r != KBurnNoErr) {
		return r;
	}
	DeferCall(serial_monitor_destroy, scope);

	r = usb_monitor_prepare(scope);
	if (r != KBurnNoErr) {
		return r;
	}

	scope->monitor_inited = true;

	dispose_list_add(scope->disposables, toDisposable(_dispose, scope));

	DeferAbort;
	return kburnWaitDeviceResume(scope);
}

void kburnWaitDevicePause(KBCTX scope) {
	debug_trace_function();

	usb_monitor_pause(scope);
	serial_monitor_pause(scope);
}
kburn_err_t kburnWaitDeviceResume(KBCTX scope) {
	debug_trace_function();

	kburn_err_t r = serial_monitor_resume(scope);
	if (r != 0) {
		return r;
	}
	return usb_monitor_resume(scope);
}
