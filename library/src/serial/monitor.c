#include "serial.h"

monitor mon = {NULL, NULL, NULL};
static bool already_init = false;

static void on_event(void *UNUSED(_), ser_dev_evt_t evt, const ser_dev_t *dev)
{
	switch (evt)
	{
	case SER_DEV_EVT_ADDED:
		debug_print("[monitor] connect: %s", dev->path);
		on_device_attach(dev->path);
		break;
	case SER_DEV_EVT_REMOVED:
		debug_print("[monitor] remove : %s", dev->path);
		kburnSerialNode *port = find_from_port_list(dev->path);
		if (port)
			destroy_port(port);
		break;
	}
}

static inline void init_list()
{
	debug_print("[monitor] init_list()");
	ser_dev_list_t *lst;

	lst = ser_dev_list_get();
	assert((lst != NULL) && sererr_last());
	ser_dev_list_t *item;

	ser_dev_list_foreach(item, lst)
	{
		debug_print("[monitor]   * %s", item->dev.path);
		on_device_attach(item->dev.path);
	}

	ser_dev_list_destroy(lst);
}

void dispose_monitor(void *UNUSED(_))
{
	global_resource_unregister(dispose_monitor, NULL);
	if (mon.instance != NULL)
	{
		kburnWaitDevicePause();
	}
}

void kburnWaitDevice(on_device_connect verify_callback, on_device_handle handler_callback)
{
	debug_print("kburnWaitDevice() [already_init=%d]", already_init);

	mon.handler_callback = handler_callback;
	mon.verify_callback = verify_callback;

	if (!already_init)
	{
		already_init = true;
		init_list();
		kburnWaitDeviceResume();
		global_resource_register(dispose_monitor, NULL);
	}
}

void kburnWaitDevicePause()
{
	debug_print("kburnWaitDevicePause() [instance=%p]", (void *)mon.instance);
	if (mon.instance != NULL)
	{
		ser_dev_monitor_stop(mon.instance);
		mon.instance = NULL;
	}
}
void kburnWaitDeviceResume()
{
	debug_print("kburnWaitDeviceResume() [instance=%p]", (void *)mon.instance);
	if (mon.instance == NULL)
		mon.instance = ser_dev_monitor_init(on_event, NULL);
}
