#include "global.h"
#include <libudev.h>
#include <stdlib.h>

#define get_number_prop(store, PROP, base)            \
	pstr = udev_device_get_property_value(dev, PROP); \
	store = pstr ? strtol(pstr, (char **)NULL, base) : 0

void driver_get_devinfo_free(kburnSerialDeviceInfo deviceInfo)
{
	if (deviceInfo.usbDriver)
		free(deviceInfo.usbDriver);
}

kburnSerialDeviceInfo driver_get_devinfo(const char *path)
{
	debug_print("driver_get_devinfo(%s)", path);

	const char *pstr;
	struct udev_list_entry *list;
	struct udev_list_entry *itr;
	kburnSerialDeviceInfo ret;
	memset(&ret, 0, sizeof(kburnSerialDeviceInfo));

	struct udev *u = udev_new();
	if (u == NULL)
		goto exit;

	/* create libudev enumerate */
	struct udev_enumerate *query = udev_enumerate_new(u);
	if (query == NULL)
	{
		goto exit;
	}

	udev_enumerate_add_match_subsystem(query, "tty");
	udev_enumerate_scan_devices(query);
	list = udev_enumerate_get_list_entry(query);

	struct udev_device *dev = NULL;
	udev_list_entry_foreach(itr, list)
	{
		dev = udev_device_new_from_syspath(u, udev_list_entry_get_name(itr));

		if (dev == NULL)
			continue;

		const char *node = udev_device_get_devnode(dev);

		if (strcmp(node, path) == 0)
		{
			debug_print("driver_get_devinfo: found device");

			pstr = udev_device_get_property_value(dev, "SUBSYSTEM");
			if (pstr && (strcmp(pstr, "tty") == 0))
			{
				ret.isTTY = true;
			}

			pstr = udev_device_get_property_value(dev, "ID_BUS");
			if (pstr && (strcmp(pstr, "usb") == 0))
			{
				ret.isUSB = true;
				get_number_prop(ret.usbIdVendor, "ID_VENDOR_ID", 16);
				get_number_prop(ret.usbIdProduct, "ID_MODEL_ID", 16);
				get_number_prop(ret.usbIdProduct, "ID_REVISION", 16);

				pstr = udev_device_get_property_value(dev, "ID_USB_DRIVER");
				if (pstr)
					ret.usbDriver = strdup(pstr);
			}

			get_number_prop(ret.major, "MAJOR", 1);
			get_number_prop(ret.minor, "MINOR", 1);

			break;
		}

		udev_device_unref(dev);
		dev = NULL;
	}

exit:
	if (query)
		udev_enumerate_unref(query);
	if (dev)
		udev_device_unref(dev);
	if (u)
		udev_unref(u);
	return ret;
}

/*
[udev]     CURRENT_TAGS = :systemd:
[udev]     DEVLINKS = /dev/serial/by-path/pci-0000:00:14.0-usb-0:4.4.2:1.0-port0 /dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0
[udev]     DEVNAME = /dev/ttyUSB0
[udev]     DEVPATH = /devices/pci0000:00/0000:00:14.0/usb1/1-4/1-4.4/1-4.4.2/1-4.4.2:1.0/ttyUSB0/tty/ttyUSB0
[udev]     ID_AUTOSUSPEND = 1
[udev]     ID_BUS = usb
[udev]     ID_MODEL = USB2.0-Serial
[udev]     ID_MODEL_ENC = USB2.0-Serial
[udev]     ID_MODEL_FROM_DATABASE = CH340 serial converter
[udev]     ID_MODEL_ID = 7523
[udev]     ID_PATH = pci-0000:00:14.0-usb-0:4.4.2:1.0
[udev]     ID_PATH_TAG = pci-0000_00_14_0-usb-0_4_4_2_1_0
[udev]     ID_PCI_CLASS_FROM_DATABASE = Serial bus controller
[udev]     ID_PCI_INTERFACE_FROM_DATABASE = XHCI
[udev]     ID_PCI_SUBCLASS_FROM_DATABASE = USB controller
[udev]     ID_REVISION = 0254
[udev]     ID_SERIAL = 1a86_USB2.0-Serial
[udev]     ID_TYPE = generic
[udev]     ID_USB_CLASS_FROM_DATABASE = Vendor Specific Class
[udev]     ID_USB_DRIVER = ch341
[udev]     ID_USB_INTERFACES = :ff0102:
[udev]     ID_USB_INTERFACE_NUM = 00
[udev]     ID_VENDOR = 1a86
[udev]     ID_VENDOR_ENC = 1a86
[udev]     ID_VENDOR_FROM_DATABASE = QinHeng Electronics
[udev]     ID_VENDOR_ID = 1a86
[udev]     MAJOR = 188
[udev]     MINOR = 0
[udev]     SUBSYSTEM = tty
[udev]     TAGS = :systemd:
[udev]     USEC_INITIALIZED = 314640517209

=============================

[udev]     CURRENT_TAGS = :systemd:
[udev]     DEVNAME = /dev/ttyS0
[udev]     DEVPATH = /devices/platform/serial8250/tty/ttyS0
[udev]     MAJOR = 4
[udev]     MINOR = 64
[udev]     SUBSYSTEM = tty
[udev]     TAGS = :systemd:
[udev]     USEC_INITIALIZED = 7529768

*/
