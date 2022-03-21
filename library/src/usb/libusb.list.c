#include "usb.h"

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

int _get_serial(struct libusb_device *dev, uint8_t iSerial, uint8_t *output, size_t buffsize)
{

	PCONST libusb_device_handle *handle;
	int r;

	r = libusb_open(dev, &handle);
	if (r != LIBUSB_SUCCESS)
	{
		debug_print("get_serial: libusb_open(): %s", libusb_strerror(r));
		return r;
	}
	r = libusb_get_string_descriptor_ascii(handle, iSerial, output, buffsize);
	if (r != LIBUSB_SUCCESS)
		debug_print("get_serial: libusb_get_string_descriptor_ascii(): %s", libusb_strerror(r));

	libusb_close(handle);

	return r;
}

struct usb_get_serial_ret usb_get_serial(struct libusb_device *dev, uint8_t *output, size_t buffsize)
{
	struct usb_get_serial_ret ret;
	struct libusb_device_descriptor desc;

	ret.error = libusb_get_device_descriptor(dev, &desc);
	if (ret.error != LIBUSB_SUCCESS)
	{
		debug_print("get_all_unopend_usb_info: libusb_get_device_descriptor(): %s", libusb_strerror(ret.error));
		return ret;
	}

	ret.vid = desc.idVendor;
	ret.pid = desc.idProduct;
	ret.error = _get_serial(dev, desc.iSerialNumber, output, buffsize);

	return ret;
}

void free_got_usb_device(libusb_device *dev)
{
	libusb_unref_device(dev);
}

libusb_device *get_usb_device(struct libusb_context *libusb, uint16_t vid, uint16_t pid, const uint8_t *serial)
{
	debug_print("get_usb_device(%d, %d, %s)", vid, pid, serial);
	libusb_device *ret = NULL;
	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(libusb, &list);
	if (cnt < 0)
	{
		debug_print("get_usb_device: libusb_get_device_list(): %s", libusb_strerror(cnt));
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

		uint8_t serialString[MAX_SERIAL_LENGTH];
		if (_get_serial(dev, desc.iSerialNumber, serialString, MAX_SERIAL_LENGTH) != LIBUSB_SUCCESS)
			continue;

		if (strcmp((const char *)serialString, (const char *)serial) == 0)
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
		debug_print("get_all_unopend_usb_info: libusb_get_device_list(): %s", libusb_strerror(cnt));
		return -1;
	}

	*ret = calloc(cnt, sizeof(kburnUsbDeviceInfo *));
	kburnUsbDeviceInfo *info = NULL;
	assert((*ret != NULL) && "memory error");

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
			debug_print("get_all_unopend_usb_info: libusb_get_device_descriptor(): %s", libusb_strerror(r));
			continue;
		}

		if (!match_device(vid, pid, &desc))
			continue;

		if (_get_serial(dev, desc.iSerialNumber, info->strSerial, MAX_SERIAL_LENGTH) != LIBUSB_SUCCESS)
			continue;

		if (usb_device_find(scope, desc.idVendor, desc.idProduct, info->strSerial) != NULL)
		{
			debug_print("  * %s - already open", info->strSerial);
			continue;
		}
		else
		{
			debug_print("  * %s", info->strSerial);
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
	struct libusb_device_descriptor desc;
	uint8_t strSerial[MAX_SERIAL_LENGTH];

	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(scope->usb->libusb, &list);
	for (int i = 0; i < cnt; i++)
	{
		libusb_device *dev = list[i];

		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
		{
			debug_print("[init/poll] libusb_get_device_descriptor(): %s", libusb_strerror(r));
			continue;
		}

		if (!match_device(scope->usb->filter.vid, scope->usb->filter.pid, &desc))
			continue;

		if (_get_serial(dev, desc.iSerialNumber, strSerial, MAX_SERIAL_LENGTH) != LIBUSB_SUCCESS)
			continue;

		if (usb_device_find(scope, desc.idVendor, desc.idProduct, strSerial) != NULL)
			continue;
		else
			debug_print("[init/poll] open: [%d:%d] %s", desc.idVendor, desc.idProduct, strSerial);
	}
	libusb_free_device_list(list, true);

	return KBurnNoErr;
}
