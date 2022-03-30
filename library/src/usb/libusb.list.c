#include "usb.h"
#include "components/call-user-handler.h"

static inline bool match_device(int vid, int pid, const struct libusb_device_descriptor *desc)
{
	if (vid != KBURN_VIDPID_FILTER_ANY && vid != desc->idVendor)
	{
		return false;
	}
	if (pid != KBURN_VIDPID_FILTER_ANY && pid != desc->idProduct)
	{
		return false;
	}
	return true;
}

void free_got_usb_device(libusb_device *dev)
{
	libusb_unref_device(dev);
}

libusb_device *get_usb_device(struct libusb_context *libusb, uint16_t vid, uint16_t pid, const uint8_t *in_path)
{
	debug_print("get_usb_device(%d, %d, %.8s)", vid, pid, in_path);
	libusb_device *ret = NULL;
	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(libusb, &list);
	if (cnt < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("get_usb_device: libusb_get_device_list()", cnt);
		return ret;
	}
	for (int i = 0; i < cnt; i++)
	{
		libusb_device *dev = list[i];
		struct libusb_device_descriptor desc;

		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
			continue;

		if (!match_device(vid, pid, &desc))
			continue;

		uint8_t path[MAX_PATH_LENGTH] = {0};
		if (usb_get_device_path(dev, path) < LIBUSB_SUCCESS)
			continue;

		if (strncmp((const char *)path, (const char *)in_path, MAX_PATH_LENGTH) == 0)
		{
			ret = dev;
			libusb_ref_device(ret);
			break;
		}
	}
	libusb_free_device_list(list, true);
	debug_print("	get_usb_device() found %p", (void *)ret);
	return ret;
}

void free_all_unopend_usb_info(kburnUsbDeviceInfo **list)
{
	for (kburnUsbDeviceInfo **itr = list; *itr != NULL; itr++)
	{
		free(*itr);
	}
}

int get_all_unopend_usb_info(KBCTX scope, int vid, int pid, kburnUsbDeviceInfo **ret)
{
	struct libusb_context *libusb = scope->usb->libusb;
	debug_print("get_all_unopend_usb_info: vid=%d, pid=%d", vid, pid);
	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(libusb, &list);
	if (cnt < 0)
	{
		debug_print_libusb_error("get_all_unopend_usb_info: libusb_get_device_list()", cnt);
		return -1;
	}

	*ret = calloc(cnt, sizeof(kburnUsbDeviceInfo *));
	kburnUsbDeviceInfo *info = NULL;
	m_assert_ptr(*ret, "memory error");

	int found = 0;
	for (int i = 0; i < cnt; i++)
	{
		libusb_device *dev = list[i];
		struct libusb_device_descriptor desc;

		if (info == NULL)
			info = calloc(1, sizeof(kburnUsbDeviceInfo));

		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
		{
			debug_print_libusb_error("get_all_unopend_usb_info: libusb_get_device_descriptor", r);
			continue;
		}

		if (!match_device(vid, pid, &desc))
			continue;

		uint8_t path[MAX_PATH_LENGTH] = {0};
		if (usb_get_device_path(dev, path) < LIBUSB_SUCCESS)
			continue;

		if (usb_device_find(scope, desc.idVendor, desc.idProduct, path) != NULL)
		{
			debug_print("  * %s - already open", usb_debug_path_string(path));
			continue;
		}
		else
		{
			debug_print("  * %s", usb_debug_path_string(path));
		}

		ret[found] = info;
		info = NULL;
		found++;
	}
	libusb_free_device_list(list, true);

	if (info)
		free(info);

	debug_print("get_all_unopend_usb_info: found %d devices.", found);

	return found;
}

kburn_err_t init_list_all_usb_devices(KBCTX scope)
{
	debug_print("[init/poll] init_list_all_usb_devices [%.4x:%.4x]", scope->usb->filter.vid, scope->usb->filter.pid);
	struct libusb_device_descriptor desc;

	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(scope->usb->libusb, &list);
	for (int i = 0; i < cnt; i++)
	{
		libusb_device *dev = list[i];

		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
		{
			debug_print_libusb_error("[init/poll] libusb_get_device_descriptor()", r);
			continue;
		}
		debug_print("[init/poll] found device: [%.4x:%.4x]", desc.idVendor, desc.idProduct);

		if (!match_device(scope->usb->filter.vid, scope->usb->filter.pid, &desc))
		{
			debug_print("[init/poll] \tnot wanted device.");
			continue;
		}

		uint8_t path[MAX_PATH_LENGTH] = {0};
		if (usb_get_device_path(dev, path) < LIBUSB_SUCCESS)
		{
			debug_print("[init/poll] \tget path failed.");
			continue;
		}

		debug_print("[init/poll] \tpath: %s", usb_debug_path_string(path));

		if (usb_device_find(scope, desc.idVendor, desc.idProduct, path) != NULL)
		{
			debug_print("[init/poll] \tdevice already opened, ignore.");
			continue;
		}
		else
		{
			debug_print("[init/poll] \topen");

			IfErrorReturn(
				open_single_usb_port(scope, dev, NULL));
		}
	}
	libusb_free_device_list(list, true);

	return KBurnNoErr;
}
