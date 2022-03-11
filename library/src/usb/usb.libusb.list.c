#include "usb.h"

static bool match_device(KBCTX scope, int vid, int pid)
{
	if (scope->usb->filter.vid != KBURN_VIDPID_FILTER_ANY && vid != scope->usb->filter.vid)
	{
		return false;
	}
	if (scope->usb->filter.pid != KBURN_VIDPID_FILTER_ANY && pid != scope->usb->filter.pid)
	{
		return false;
	}
	return true;
}

kburn_err_t init_list_usb(KBCTX scope)
{
	unsigned char string[128];
	memset(string, 0, sizeof(string));
	libusb_device **list;

	/* 枚举设备 */
	ssize_t cnt = libusb_get_device_list(scope->usb->libusb, &list);
	if (cnt < 0)
	{
		debug_print("get device list failed: %s", libusb_strerror(cnt));
		return KBURN_ERROR_KIND_USB | cnt;
	}

	for (int i = 0; i < cnt; i++)
	{
		libusb_device *dev = list[i];
		struct libusb_device_descriptor desc;

		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
		{
			debug_print("failed to get device descriptor: %s", libusb_strerror(r));
			return KBURN_ERROR_KIND_USB | r;
		}

		if (match_device(scope, desc.idVendor, desc.idProduct))
		{
			/* 如果匹配到设备就建立设备节点 */
			usbdev_node *current = (usbdev_node *)malloc(sizeof(usbdev_node));
			memset(string, 0, sizeof(string));
			r = libusb_open(dev, &current->handle);
			if (r < 0)
			{
				printf("Open device failed: %s\n", libusb_strerror((enum libusb_error)r));
				continue;
			}

			if (libusb_get_string_descriptor_ascii(current->handle, desc.iSerialNumber, (unsigned char *)string, sizeof(string)) > 0)
			{
				printf("\nString (0x%02X): \"%s\"\n", desc.iSerialNumber, string);
				strcpy((char *)current->stringID, (char *)string);
			}

			// r = getDevEndpoint(dev, current);
			// if (r < 0)
			// {
			// 	printf("Get endpoint failed");
			// }

			/* 声明接口,记得所有数据传输完成之后要释放*/
			r = libusb_claim_interface(current->handle, 0);
			if (r < 0)
			{
				printf("Claiming interface Failed.%s\n", libusb_strerror((enum libusb_error)r));
			}
		}
	}
	libusb_free_device_list(list, true);

	return KBurnNoErr;
}
