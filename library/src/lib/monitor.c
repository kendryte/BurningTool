#include "serial.h"
#include "usb.h"

static DECALRE_DISPOSE(_dispose, kburnContext)
{
	if (context->monitor_inited)
	{
		usb_monitor_pause(context);
		serial_monitor_pause(context);

		context->monitor_inited = false;
		usb_subsystem_deinit(context);
		serial_subsystem_deinit(context);
	}
}
DECALRE_DISPOSE_END()

kburn_err_t kburnWaitDeviceInitStart(KBCTX scope)
{
	debug_print("kburnWaitDeviceInitStart() [already_init=%d]", scope->monitor_inited);
	kburn_err_t r;

	if (!scope->monitor_inited)
	{
		scope->monitor_inited = true;
		global_resource_register(scope, _dispose, scope);

		r = serial_subsystem_init(scope);
		if (r != 0)
			return r;
		r = usb_subsystem_init(scope);
		if (r != 0)
			return r;
	}

	return kburnWaitDeviceResume(scope);
}

void kburnWaitDevicePause(KBCTX scope)
{
	debug_print("kburnWaitDevicePause()");

	serial_monitor_pause(scope);
	usb_monitor_pause(scope);
}
kburn_err_t kburnWaitDeviceResume(KBCTX scope)
{
	debug_print("kburnWaitDeviceResume()");

	kburn_err_t r = serial_monitor_resume(scope);
	if (r != 0)
		return r;
	return usb_monitor_resume(scope);
}
