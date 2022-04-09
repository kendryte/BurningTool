#include "basic/errors.h"
#include "device.h"
#include "isp.low.h"
#include "private-types.h"
#include "canaan-burn/exported/usb.isp.h"
#include <string.h>
#include <time.h>

static inline usbIspCommandTaget convertTarget(kburnUsbIspCommandTaget i) {
	switch (i) {
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

kburn_err_t kburnUsbIspLedControl(kburnDeviceNode *node, uint8_t pin, struct kburnColor color) {
	debug_trace_function();
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_WRITE_DEVICE,
		.target = USB_ISP_COMMAND_TARGET_LED,
		.led.led_io = pin,
		.led.red = color.red,
		.led.green = color.green,
		.led.blue = color.blue,
	};
	uint32_t expected_tag = rand();

	IfErrorReturn(usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, 0, expected_tag));

	return usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag);
}

kburn_err_t kburnUsbIspGetMemorySize(kburnDeviceNode *node, kburnUsbIspCommandTaget target, kburnDeviceMemorySizeInfo *out_dev_info) {
	debug_trace_function();

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_READ_CAPACITY,
		.target = convertTarget(target),
	};
	uint32_t expected_tag = rand();
	kburn_stor_address_t buffer[2];

	IfErrorReturn(
		usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_IN, sizeof(buffer), expected_tag));

	IfErrorReturn(usb_lowlevel_transfer(node->usb, USB_READ, &buffer, sizeof(buffer)));

	IfErrorReturn(usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag));

	kburn_stor_address_t last_block_address = be32toh(buffer[0]);
	uint32_t block_size = be32toh(buffer[1]);
	uint64_t storage_size = (uint64_t)(out_dev_info->last_block_address + 1) * out_dev_info->block_size;
	*out_dev_info = (kburnDeviceMemorySizeInfo){
		.device = target,
		.base_address = 0,
		.last_block_address = last_block_address,
		.block_size = block_size,
		.storage_size = storage_size,
		.block_count = storage_size / block_size,
	};

	return KBurnNoErr;
}

static kburn_err_t check_input(kburnDeviceNode *node, kburn_stor_block_t nblock, int32_t length, const kburnDeviceMemorySizeInfo dev_info) {
	if (length % dev_info.block_size != 0) {
		kburn_err_t r = make_error_code(KBURN_ERROR_KIND_COMMON, KBurnSizeNotAlign);
		set_kb_error(node, r, "buffer length (%d) is not align to block (size=%d)", length, dev_info.block_size);
		return r;
	}
	kburn_stor_address_t over_last_block = nblock + length / dev_info.block_size;
	if (over_last_block - 1 > dev_info.block_count) {
		kburn_err_t r = make_error_code(KBURN_ERROR_KIND_COMMON, KBurnAddressOverflow);
		set_kb_error(node, r, "ending block (%d) is larger than the device (%d)", over_last_block - 1, dev_info.block_count);
		return r;
	}

	return KBurnNoErr;
}

static kburn_err_t read_normalized(
	kburnDeviceNode *node, kburnUsbIspCommandTaget device, kburn_stor_address_t start_addr, uint32_t block_count, void *buffer, uint32_t length) {
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_READ_BURN,
		.target = convertTarget(device),
		.burn.address = htobe32(start_addr),
		.burn.block_count = htobe16(block_count),
	};
	uint32_t expected_tag = rand();

	IfErrorReturn(
		usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_IN, length, expected_tag));

	IfErrorReturn(usb_lowlevel_transfer(node->usb, USB_READ, buffer, length));

	IfErrorReturn(usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag));

	return KBurnNoErr;
}

static kburn_err_t write_normalized(
	kburnDeviceNode *node, kburnUsbIspCommandTaget device, kburn_stor_address_t start_addr, uint32_t block_count, void *buffer, uint32_t length) {
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_WRITE_BURN,
		.target = convertTarget(device),
		.burn.address = htobe32(start_addr),
		.burn.block_count = htobe16(block_count),
	};
	uint32_t expected_tag = rand();

	IfErrorReturn(
		usb_lowlevel_command_send(node->usb->handle, node->usb->deviceInfo.endpoint_out, request, LIBUSB_ENDPOINT_OUT, length, expected_tag));

	IfErrorReturn(usb_lowlevel_transfer(node->usb, USB_WRITE, buffer, length));

	IfErrorReturn(usb_lowlevel_status_read(node->usb->handle, node->usb->deviceInfo.endpoint_in, expected_tag));

	return KBurnNoErr;
}

kburn_err_t kburnUsbIspWriteChunk(
	kburnDeviceNode *node, const kburnDeviceMemorySizeInfo dev_info, kburn_stor_block_t start_block, void *buffer, uint32_t length) {
	debug_trace_function();
	IfErrorReturn(check_input(node, start_block, length, dev_info));

	uint32_t block_count = length / dev_info.block_size;
	uint32_t start_addr = (start_block * dev_info.block_size) + dev_info.base_address;

	return write_normalized(node, dev_info.device, start_addr, block_count, buffer, length);
}

kburn_err_t
kburnUsbIspReadChunk(kburnDeviceNode *node, const kburnDeviceMemorySizeInfo dev_info, kburn_stor_block_t start_block, uint32_t length, void *buffer) {
	debug_trace_function();
	IfErrorReturn(check_input(node, start_block, length, dev_info));

	uint32_t block_count = length / dev_info.block_size;
	uint32_t start_addr = (start_block * dev_info.block_size) + dev_info.base_address;

	return read_normalized(node, dev_info.device, start_addr, block_count, buffer, length);
}
