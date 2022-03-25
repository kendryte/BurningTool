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

kburn_err_t kburnStartWaitingDevices(KBCTX scope)
{
	debug_print("kburnStartWaitingDevices() [already_init=%d]", scope->monitor_inited);
	if (scope->monitor_inited)
		return KBurnNoErr;

	kburn_err_t r;

	if (!scope->usb->libusb)
		usb_subsystem_init(scope);

	if (!scope->serial->subsystem_inited)
		serial_subsystem_init(scope);

	r = serial_monitor_prepare(scope);
	if (r != KBurnNoErr)
		return r;

	r = usb_monitor_prepare(scope);
	if (r != KBurnNoErr)
	{
		serial_monitor_destroy(scope);
		return r;
	}

	scope->monitor_inited = true;

	global_resource_register(scope, _dispose, scope);

	return kburnWaitDeviceResume(scope);
}

void kburnWaitDevicePause(KBCTX scope)
{
	debug_print("kburnWaitDevicePause()");

	usb_monitor_pause(scope);
	serial_monitor_pause(scope);
}
kburn_err_t kburnWaitDeviceResume(KBCTX scope)
{
	debug_print("kburnWaitDeviceResume()");

	kburn_err_t r = serial_monitor_resume(scope);
	if (r != 0)
		return r;
	return usb_monitor_resume(scope);
}
