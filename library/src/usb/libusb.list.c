#include "descriptor.h"
#include "device.h"
#include "lifecycle.h"
#include "math.h"
#include "private-types.h"
#include "subsystem.h"
#include "components/call-user-handler.h"
#include "components/device-link-list.h"
#include "canaan-burn/canaan-burn.h"

static inline bool match_device(int vid, int pid, const struct libusb_device_descriptor *desc) {
	if (vid != KBURN_VIDPID_FILTER_ANY && vid != desc->idVendor) {
		return false;
	}
	if (pid != KBURN_VIDPID_FILTER_ANY && pid != desc->idProduct) {
		return false;
	}
	return true;
}

kburn_err_t init_list_all_usb_devices(KBCTX scope) {
	debug_trace_function("%.4x:%.4x", scope->usb->settings.vid, scope->usb->settings.pid);
	struct libusb_device_descriptor desc;

	libusb_device **list;
	ssize_t r = libusb_get_device_list(scope->usb->libusb, &list);
	if (!check_libusb(r)) {
		debug_print_libusb_error("libusb_get_device_list()", r);
		return r;
	}
	for (int i = 0; i < r; i++) {
		libusb_device *dev = list[i];

		r = libusb_get_device_descriptor(dev, &desc);
		if (!check_libusb(r)) {
			debug_print_libusb_error("[init/poll] libusb_get_device_descriptor()", r);
			continue;
		}
		debug_print(KBURN_LOG_DEBUG, "[init/poll] found device: [%.4x:%.4x]", desc.idVendor, desc.idProduct);

		if (!match_device(subsystem_settings.vid, subsystem_settings.pid, &desc)) {
			debug_print(KBURN_LOG_DEBUG, "[init/poll] \tnot wanted device.");
			continue;
		}

		uint8_t path[MAX_USB_PATH_LENGTH] = {0};
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

size_t list_usb_ports(struct libusb_context *libusb, kburnUsbDeviceInfoSlice *out, size_t max_size) {
	debug_trace_function();

	m_assert_ptr(libusb, "use not init libusb instance");

	libusb_device **list;
	ssize_t count = libusb_get_device_list(libusb, &list);
	if (!check_libusb(count)) {
		debug_print_libusb_error("libusb_get_device_list()", count);
		return count;
	}
	debug_print(KBURN_LOG_INFO, COLOR_FMT("libusb_get_device_list") ": " FMT_SIZET, RED, (size_t)count);

	size_t index = 0;
	for (size_t i = 0; i < (size_t)count; i++) {
		libusb_device *dev = list[i];
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (!check_libusb(r)) {
			debug_print_libusb_error("libusb_get_device_descriptor()", r);
			continue;
		}

		if (index >= max_size) {
			index++;
			continue;
		}

		out[index].idProduct = desc.idProduct;
		out[index].idVendor = desc.idVendor;

		r = usb_get_device_path(dev, &out[index].path[0]);
		if (!kburn_not_error(r)) {
			debug_print_libusb_error("usb_get_device_path()", r);
		}

		index++;
	}
	libusb_free_device_list(list, true);

	return index;
}
