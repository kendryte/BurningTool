#include "isp.h"
#include "context.h"
#include "basic/disposable.h"
#include "basic/errors.h"
#include "basic/resource-tracker.h"
#include "descriptor.h"
#include "device.h"
#include "isp.low.h"
#include "private-types.h"

kburn_err_t usb_device_hello(kburnDeviceNode *node) {
	debug_trace_function();

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_HELLO,
	};

	uint32_t expected_tag = rand();

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	if (!usb_lowlevel_command_send(node->usb, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag)) {
		return node->error->code;
	}
	/* 数据阶段 */

	/* 没有数据阶段了*/

	/* 状态阶段 */
	if (!usb_lowlevel_status_read(node->usb, node->usb->deviceInfo.endpoint_in, expected_tag)) {
		kburnErrorDesc rr = kburnSplitErrorCode(node->error->code);
		if (rr.kind == KBURN_ERROR_KIND_COMMON && rr.code == KBurnUsbErrorSense) {
			usb_lowlevel_error_read(node->usb, node->usb->deviceInfo.endpoint_in, node->usb->deviceInfo.endpoint_out);
		}
		return node->error->code;
	}
	return KBurnNoErr;
}

static bool usb_device_serial_put_str4(kburnDeviceNode *node, const uint8_t ch[4]) {
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_WRITE_DEVICE,
		.target = USB_ISP_COMMAND_TARGET_UART,
	};

	request.uart[0] = '\xff';
	for (size_t i = 0; i < 4; i++) {
		request.uart[i + 1] = ch[i];
		request.uart[5] += __builtin_popcount(ch[i]);
	}

	uint32_t expected_tag = rand();

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	ifNotReturnFalse(usb_lowlevel_command_send(node->usb, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag));

	return usb_lowlevel_status_read(node->usb, node->usb->deviceInfo.endpoint_in, expected_tag);
}

bool usb_device_serial_print(kburnDeviceNode *node, const uint8_t *buff, size_t buff_size) {
	for (size_t i = 0; i < buff_size; i += 4) {
		if (i + 4 > buff_size) {
			uint8_t nbuff[4] = {0};
			memcpy(nbuff, buff + i, buff_size - i);
			ifNotReturnFalse(usb_device_serial_put_str4(node, nbuff));
		} else {
			ifNotReturnFalse(usb_device_serial_put_str4(node, buff + i));
		}
	}
	return true;
}
