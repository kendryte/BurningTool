#include <time.h>
#include <string.h>
#include "usb.h"
#include "isp.low.h"

static inline usbIspCommandTaget convertTarget(kburnUsbIspCommandTaget i)
{
	switch (i)
	{
	case KBURN_USB_ISP_EMMC:
		return USB_ISP_COMMAND_TARGET_EMMC;
	case KBURN_USB_ISP_SDCARD:
		return USB_ISP_COMMAND_TARGET_SDCARD;
	case KBURN_USB_ISP_NAND:
		return USB_ISP_COMMAND_TARGET_NAND;
	case KBURN_USB_ISP_OTP:
		return USB_ISP_COMMAND_TARGET_OTP;
	}
	m_abort("invalid usb isp command target: %d", i);
}

kburn_err_t kburnUsbIspLedControl(kburnDeviceNode *node, uint8_t pin, struct kburnColor color)
{
	debug_print("%s:", __func__);
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_WRITE_DEVICE,
		.target = USB_ISP_COMMAND_TARGET_LED,
		.led.led_io = pin,
		.led.red = color.red,
		.led.green = color.green,
		.led.blue = color.blue,
	};
	uint32_t expected_tag = rand();

	IfErrorReturn(
		usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag));

	return usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag);
}

kburn_err_t kburnUsbIspGetMemorySize(kburnDeviceNode *node, kburnUsbIspCommandTaget target, kburnDeviceMemorySizeInfo *out_dev_info)
{
	debug_print("%s:", __func__);
#define READ_CAPACITY_LENGTH 0x08

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_READ_CAPACITY,
		.target = convertTarget(target),
	};
	uint32_t expected_tag = rand();

	IfErrorReturn(
		usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_IN, READ_CAPACITY_LENGTH, expected_tag));

	uint8_t buffer[READ_CAPACITY_LENGTH];

	IfErrorReturn(
		usb_lowlevel_transfer(node->usb, USB_READ, buffer, READ_CAPACITY_LENGTH));

	IfErrorReturn(
		usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag));

	out_dev_info->max_block_addr = uchar2uint(&buffer[0]);
	out_dev_info->block_size = uchar2uint(&buffer[4]);
	out_dev_info->device_size = (uint64_t)(out_dev_info->max_block_addr + 1) * out_dev_info->block_size;

	return KBurnNoErr;
}

static kburn_err_t check_input(kburnDeviceNode *node, uint64_t address, int32_t length, const kburnDeviceMemorySizeInfo *dev_info)
{
	if (length % dev_info->block_size != 0)
	{
		kburn_err_t r = make_error_code(KBURN_ERROR_KIND_COMMON, KBurnSizeNotAlign);
		set_kb_error(node, r, "buffer length (%d) is not align to block (size=%d)", length, dev_info->block_size);
		return r;
	}
	if (address % dev_info->block_size != 0)
	{
		kburn_err_t r = make_error_code(KBURN_ERROR_KIND_COMMON, KBurnAddressNotAlign);
		set_kb_error(node, r, "address (%lX) is not align to block (size=%d)", address, dev_info->block_size);
		return r;
	}
	return KBurnNoErr;
}

kburn_err_t kburnUsbIspWriteData(kburnDeviceNode *node, kburnUsbIspCommandTaget target, uint64_t address, unsigned char *buffer, uint32_t length, const kburnDeviceMemorySizeInfo *dev_info)
{
	debug_print("%s:", __func__);
	IfErrorReturn(
		check_input(node, address, length, dev_info));

	uint32_t block_num = length / dev_info->block_size;
	uint32_t block_addr = (uint32_t)(address / dev_info->block_size);

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_WRITE_BURN,
		.target = convertTarget(target),
		.burn.address = htobe32(block_addr),
		.burn.block_count = htobe16(block_num),
	};
	uint32_t expected_tag = rand();

	IfErrorReturn(
		usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, length, expected_tag));

	IfErrorReturn(
		usb_lowlevel_transfer(node->usb, USB_WRITE, buffer, length));

	IfErrorReturn(
		usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag));

	return KBurnNoErr;
}

kburn_err_t kburnUsbIspReadData(kburnDeviceNode *node, kburnUsbIspCommandTaget target, uint64_t address, uint32_t length, unsigned char *buffer, const kburnDeviceMemorySizeInfo *dev_info)
{
	debug_print("%s:", __func__);
	IfErrorReturn(
		check_input(node, address, length, dev_info));

	uint32_t block_num = length / dev_info->block_size;
	uint32_t block_addr = (uint32_t)(address / dev_info->block_size);

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_READ_BURN,
		.target = convertTarget(target),
		.burn.address = htobe32(block_addr),
		.burn.block_count = htobe16(block_num),
	};
	uint32_t expected_tag = rand();

	IfErrorReturn(
		usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_IN, length, expected_tag));

	IfErrorReturn(
		usb_lowlevel_transfer(node->usb, USB_READ, buffer, length));

	IfErrorReturn(
		usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag));

	return KBurnNoErr;
}
