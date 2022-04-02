#include "basic/errors.h"
#include "isp.low.h"
#include "usb.h"

kburn_err_t usb_device_hello(kburnDeviceNode *node) {
	debug_trace_function();

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_HELLO,
	};

	int r;
	uint32_t expected_tag = rand();

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag);
	if (r < LIBUSB_SUCCESS) {
		copy_last_libusb_error(node, r);
		return make_error_code(KBURN_ERROR_KIND_USB, r);
	}
	/* 数据阶段 */

	/* 没有数据阶段了*/

	/* 状态阶段 */
	kburn_err_t e = usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag);
	if (e != KBurnNoErr) {
		kburnErrorDesc rr = kburnSplitErrorCode(e);
		if (rr.kind == KBURN_ERROR_KIND_COMMON && rr.code == KBurnUsbErrorSense) {
			usb_lowlevel_error_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, node->usb->deviceInfo.endpoint_out);
		}
		copy_last_libusb_error(node, e);
		return e;
	}
	return KBurnNoErr;
}

static kburn_err_t usb_device_serial_put_str4(kburnDeviceNode *node, const uint8_t ch[4]) {
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_WRITE_DEVICE,
		.target = USB_ISP_COMMAND_TARGET_UART,
	};

	request.uart[0] = '\xff';
	for (size_t i = 0; i < 4; i++) {
		request.uart[i + 1] = ch[i];
		request.uart[5] += __builtin_popcount(ch[i]);
	}

	int r;
	uint32_t expected_tag = rand();

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag);
	if (r != KBurnNoErr)
		return r;

	return usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag);
}

kburn_err_t usb_device_serial_print(kburnDeviceNode *node, const uint8_t *buff, size_t buff_size) {
	for (size_t i = 0; i < buff_size; i += 4) {
		kburn_err_t e;
		if (i + 4 > buff_size) {
			uint8_t nbuff[4] = {0};
			memcpy(nbuff, buff + i, buff_size - i);
			e = usb_device_serial_put_str4(node, nbuff);
		} else {
			e = usb_device_serial_put_str4(node, buff + i);
		}
		if (e != KBurnNoErr)
			return e;
	}
	return KBurnNoErr;
}
