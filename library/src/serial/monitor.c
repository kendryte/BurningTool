#include "serial.h"

static void on_event(void *ctx, ser_dev_evt_t evt, const ser_dev_t *dev)
{
	KBCTX scope = ctx;
	switch (evt)
	{
	case SER_DEV_EVT_ADDED:
		debug_print("[monitor] connect: %s", dev->path);
		on_serial_device_attach(scope, dev->path);
		break;
	case SER_DEV_EVT_REMOVED:
		debug_print("[monitor] remove : %s", dev->path);
		kburnDeviceNode *device = get_device_by_serial_port_path(scope, dev->path);
		if (device)
			destroy_serial_port(scope, device);

		break;
	}
}

void kburnOnSerialConnect(KBCTX scope, on_device_connect verify_callback, void *ctx)
{
	scope->serial->verify_callback = verify_callback;
	scope->serial->verify_callback_ctx = ctx;
}
void kburnOnSerialConfirm(KBCTX scope, on_device_handle handler_callback, void *ctx)
{
	scope->serial->handler_callback = handler_callback;
	scope->serial->handler_callback_ctx = ctx;
}

static void first_init_list(KBCTX scope, const bool *const quit)
{
	debug_print("[monitor] init_list()");
	ser_dev_list_t *lst;

	lst = ser_dev_list_get();
	if (!lst)
	{
		debug_print("serial port list get failed: %s", sererr_last());
		return;
	}

	ser_dev_list_t *item;

	ser_dev_list_foreach(item, lst)
	{
		if (*quit)
			break;

		if (prefix("/dev/ttyS", item->dev.path))
		{
			continue;
		}
		debug_print("[monitor]   * %s", item->dev.path);
		on_serial_device_attach(scope, item->dev.path);
	}

	ser_dev_list_destroy(lst);
}

void serial_monitor_destroy(KBCTX scope)
{
	debug_print("serial_monitor_destroy()");
	if (!scope->serial->monitor_prepared)
		return;
	if (scope->serial->monitor_instance)
	{
		ser_dev_monitor_stop(scope->serial->monitor_instance);
		scope->serial->monitor_instance = NULL;
	}

	if (scope->serial->init_list_thread)
	{
		thread_tell_quit(scope->serial->init_list_thread);
		thread_wait_quit(scope->serial->init_list_thread);
		scope->serial->init_list_thread = NULL;
	}

	scope->serial->monitor_prepared = false;
}

kburn_err_t serial_monitor_prepare(KBCTX scope)
{
	debug_print("serial_monitor_prepare()");
	if (scope->serial->monitor_prepared)
		return KBurnNoErr;
	scope->serial->monitor_prepared = true;

	if (scope->serial->monitor_instance)
	{
		debug_print("\talready inited.");
		return KBurnNoErr;
	}

	thread_create("serial init list", first_init_list, scope, &scope->serial->init_list_thread);

	return KBurnNoErr;
}

void serial_monitor_pause(KBCTX scope)
{
	debug_print("serial_monitor_pause() [instance=%p]", (void *)scope->serial->monitor_instance);
	if (scope->serial->monitor_instance)
	{
		ser_dev_monitor_stop(scope->serial->monitor_instance);
		scope->serial->monitor_instance = NULL;
	}
}

kburn_err_t serial_monitor_resume(KBCTX scope)
{
	debug_print("serial_monitor_resume() [instance=%p]", (void *)scope->serial->monitor_instance);
	if (scope->serial->monitor_instance == NULL)
		scope->serial->monitor_instance = ser_dev_monitor_init(on_event, scope);

	if (scope->serial->monitor_instance == NULL)
	{
		debug_print("ser_dev_monitor_init fail");
		return KBURN_ERROR_KIND_COMMON | KBurnSerialMonitorFailStart;
	}

	return KBurnNoErr;
}
