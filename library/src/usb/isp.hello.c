#include "usb.h"

kburn_err_t usb_device_hello(kburnDeviceNode *node)
{
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_HELLO,
	};

	int r;
	uint32_t expected_tag = rand();

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag);
	if (r < LIBUSB_SUCCESS)
	{
		return KBURN_ERROR_KIND_USB | r;
	}
	/* 数据阶段 */

	/* 没有数据阶段了*/

	/* 状态阶段 */
	kburn_err_t e = usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag);
	kburnErrorDesc rr = kburnSplitErrorCode(e);
	if (rr.kind == KBURN_ERROR_KIND_COMMON && rr.code == KBurnUsbErrorSense)
	{
		usb_lowlevel_error_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, node->usb->deviceInfo.endpoint_out);
		return e;
	}
	return KBurnNoErr;
}
kburn_err_t usb_device_serial_bind(kburnDeviceNode *node)
{
	debug_print("usb_device_serial_bind:");
	alloc_new_bind_id(node);
	debug_print("	bind id = %d", node->bind_id);

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_WRITE_DEVICE,
		.target = USB_ISP_COMMAND_TARGET_UART,
		.uart = node->bind_id,
	};

	int r;
	uint32_t expected_tag = rand();

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag);
	if (r < LIBUSB_SUCCESS)
	{
		return KBURN_ERROR_KIND_USB | r;
	}
	/* 数据阶段 */

	/* 没有数据阶段了*/

	/* 状态阶段 */
	kburn_err_t e = usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag);
	kburnErrorDesc rr = kburnSplitErrorCode(e);
	if (rr.kind == KBURN_ERROR_KIND_COMMON && rr.code == KBurnUsbErrorSense)
	{
		usb_lowlevel_error_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, node->usb->deviceInfo.endpoint_out);
		return e;
	}
	return KBurnNoErr;
}
