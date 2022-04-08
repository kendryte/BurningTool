#include <stdlib.h>
#include <iostream>
#include <libudev.h>
#include <string.h>

using namespace std;

void testDevice(const char *path) {
	cout << "testDevice: " << path << endl;
	struct udev_list_entry *list;
	struct udev_list_entry *itr;
	struct udev *u = udev_new();
	struct udev_device *dev = NULL;
	struct udev_enumerate *query;

	if (u == NULL)
		goto exit;

	query = udev_enumerate_new(u);
	if (query == NULL) {
		goto exit;
	}

	udev_enumerate_scan_devices(query);
	list = udev_enumerate_get_list_entry(query);

	udev_list_entry_foreach(itr, list) {
		dev = udev_device_new_from_syspath(u, udev_list_entry_get_name(itr));

		if (dev == NULL)
			continue;

		const char *node = udev_device_get_devnode(dev);
		if (node) {
			if (strcmp(node, path) == 0) {
				cout << "found device!" << endl;
				break;
			}
		}

		udev_device_unref(dev);
		dev = NULL;
	}

	list = udev_device_get_properties_list_entry(dev);
	udev_list_entry_foreach(itr, list) { cout << "[udev]\t" << udev_list_entry_get_name(itr) << " = " << udev_list_entry_get_value(itr) << endl; }

exit:
	if (query)
		udev_enumerate_unref(query);
	if (dev)
		udev_device_unref(dev);
	if (u)
		udev_unref(u);
}
