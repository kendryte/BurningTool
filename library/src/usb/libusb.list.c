#include "components/call-user-handler.h"
#include "components/device-link-list.h"
#include "descriptor.h"
#include "lifecycle.h"
#include "private-types.h"

static inline bool match_device(int vid, int pid, const struct libusb_device_descriptor *desc) {
	if (vid != KBURN_VIDPID_FILTER_ANY && vid != desc->idVendor) {
		return false;
	}
	if (pid != KBURN_VIDPID_FILTER_ANY && pid != desc->idProduct) {
		return false;
	}
	return true;
}

void free_got_usb_device(libusb_device *dev) { libusb_unref_device(dev); }

libusb_device *get_usb_device(struct libusb_context *libusb, uint16_t vid, uint16_t pid, const uint8_t *in_path) {
	debug_trace_function("%d, %d, %.8s", vid, pid, in_path);
	libusb_device *ret = NULL;
	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(libusb, &list);
	if (cnt < LIBUSB_SUCCESS) {
		debug_print_libusb_error("get_usb_device: libusb_get_device_list()", cnt);
		return ret;
	}
	for (int i = 0; i < cnt; i++) {
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

		if (strncmp((const char *)path, (const char *)in_path, MAX_PATH_LENGTH) == 0) {
			ret = dev;
			libusb_ref_device(ret);
			break;
		}
	}
	libusb_free_device_list(list, true);
	debug_print(KBURN_LOG_DEBUG, "	get_usb_device() found %p", (void *)ret);
	return ret;
}

kburn_err_t init_list_all_usb_devices(KBCTX scope) {
	debug_trace_function("%.4x:%.4x", scope->usb->filter.vid, scope->usb->filter.pid);
	struct libusb_device_descriptor desc;

	libusb_device **list;
	ssize_t cnt = libusb_get_device_list(scope->usb->libusb, &list);
	for (int i = 0; i < cnt; i++) {
		libusb_device *dev = list[i];

		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			debug_print_libusb_error("[init/poll] libusb_get_device_descriptor()", r);
			continue;
		}
		debug_print(KBURN_LOG_DEBUG, "[init/poll] found device: [%.4x:%.4x]", desc.idVendor, desc.idProduct);

		if (!match_device(scope->usb->filter.vid, scope->usb->filter.pid, &desc)) {
			debug_print(KBURN_LOG_DEBUG, "[init/poll] \tnot wanted device.");
			continue;
		}

		uint8_t path[MAX_PATH_LENGTH] = {0};
		if (usb_get_device_path(dev, path) < LIBUSB_SUCCESS) {
			debug_print(KBURN_LOG_ERROR, "[init/poll] \tget path failed.");
			continue;
		}

		debug_print(KBURN_LOG_DEBUG, "[init/poll] \tpath: %s", usb_debug_path_string(path));

		if (get_device_by_usb_port_path(scope, desc.idVendor, desc.idProduct, path) != NULL) {
			debug_print(KBURN_LOG_DEBUG, "[init/poll] \tdevice already opened, ignore.");
			continue;
		} else {
			debug_print(KBURN_LOG_DEBUG, "[init/poll] \topen");

			IfErrorReturn(open_single_usb_port(scope, dev, true, NULL));
		}
	}
	libusb_free_device_list(list, true);

	return KBurnNoErr;
}
