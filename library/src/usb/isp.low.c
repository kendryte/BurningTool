#include "isp.low.h"
#include "basic/errors.h"
#include "basic/sleep.h"
#include "device.h"
#include "isp.h"
#include "private-types.h"
#include <stdlib.h>
#include <string.h>

#define RETRY_MAX 5
#define REQUEST_SENSE_LENGTH 0x12

static const int usb_timeout = 10000;

typedef struct usbIspRequestPacket // cbw
{
	uint8_t signature[4];
	uint32_t operation_tag;
	uint32_t transfer_data_length;
	uint8_t flags;
	uint8_t __lun;
	uint8_t _body_length;
	usbIspCommandPacket body;
	uint8_t reserved[7];
} __attribute__((__packed__)) usbIspRequestPacket;
// checker: char (*__)[sizeof(usbIspRequestPacket)] = 1;
_Static_assert(sizeof(usbIspRequestPacket) == 31, "cbw packet must 31bytes");

typedef struct usbIspResponsePacket // CSW
{
	uint8_t signature[4];
	uint32_t operation_tag;
	uint32_t __data_residue;
	uint8_t return_value; // bool
} __attribute__((__packed__)) usbIspResponsePacket;
// char (*__)[sizeof(usbIspResponsePacket)] = 1;
_Static_assert(sizeof(usbIspResponsePacket) == 13, "cbw packet must 13bytes");

static inline usbIspRequestPacket
create_request(uint32_t operation_tag, uint32_t transfer_data_length, uint8_t flags, const usbIspCommandPacket body) {
	usbIspRequestPacket r = {
		.signature = "USBC",
		.operation_tag = operation_tag,
		.transfer_data_length = transfer_data_length,
		.flags = flags,
		.__lun = 0,
		._body_length = 9,
		.body = body,
		.reserved = {0xFF},
	};

	r.body.command = htobe16(r.body.command);

	return r;
}

static bool
retry_libusb_bulk_transfer(kburnUsbDeviceNode *node, unsigned char endpoint, void *data, int length, int *actual_length, unsigned int timeout) {
	int r = LIBUSB_ERROR_OTHER;
	for (int retry = 0; retry < RETRY_MAX; retry++) {
		// 传输长度必须正好是31个字节
		r = libusb_bulk_transfer(node->handle, endpoint, data, length, actual_length, timeout);
		if (r == LIBUSB_SUCCESS) {
			return true;
		}

		if (r == LIBUSB_ERROR_PIPE) {
			libusb_clear_halt(node->handle, endpoint);
			continue;
		} else if (r == LIBUSB_ERROR_BUSY) {
			debug_print_libusb_error("libusb_bulk_transfer", r);
			do_sleep(500);
			continue;
		} else {
			break;
		}
	}
	set_node_error_with_log(r, "libusb_bulk_transfer");
	return false;
}

/************************************************************
Function: usb_lowlevel_command_send
Description: 发送CWB指令,一共31个字节。
param1 : libusb_device_handle *handle          usb设备句柄
param2 : uint8_t endpoint                      端点I/O
param3 : uint8_t lun                           lun
param4 : uint8_t *cdb                          CDB字段
param5 : uint8_t direction                     传输方向
param6 : int data_length                       数据长度（字节）
param6 : operation_index                       命令id（随机数）
*************************************************************/
bool usb_lowlevel_command_send(
	kburnUsbDeviceNode *node, uint8_t endpoint_out, const usbIspCommandPacket cdb, uint8_t direction, int data_length, uint32_t operation_index) {
	debug_trace_function("%d", operation_index);

	m_assert_ptr(node->handle, "handle pointer is null!");
	m_assert0(endpoint_out & LIBUSB_ENDPOINT_IN, "cannot send command on IN endpoint"); /* 校验端点的传输方向 */

	usbIspRequestPacket cbw = create_request(operation_index, data_length, direction, cdb);

	print_buffer(KBURN_LOG_BUFFER, "cbw", (void *)&cbw, sizeof(usbIspRequestPacket));

	int written_size = 0;
	if (!retry_libusb_bulk_transfer(node, endpoint_out, &cbw, sizeof(usbIspRequestPacket), &written_size, usb_timeout)) {
		return false;
	}

	debug_print(KBURN_LOG_TRACE, "      %d bytes sent", written_size);

	return true;
}

static bool is_packet_type_csw(const void *buff) {
	return memcmp(buff, "USBS", 4) == 0;
}

static bool csw_status_parse(kburnUsbDeviceNode *node, const void *buff, uint32_t expected_operation_index) {
	if (!is_packet_type_csw(buff)) {
		set_error(node, KBURN_ERROR_KIND_COMMON, KBurnUsbReadIndexMismatch, "mismatched signature: %.4s (expected USBS)", (char *)buff);
		return false;
	}

	const usbIspResponsePacket *csw = buff;

	if (expected_operation_index && csw->operation_tag != expected_operation_index) {
		set_error(
			node, KBURN_ERROR_KIND_COMMON, KBurnUsbReadIndexMismatch, "mismatched tags: %d (expected %d)", csw->operation_tag,
			expected_operation_index);
		return false;
	}

	if (csw->return_value) {
		// 状态值为1说明有错误出现，应该使用GetSense获取错误原因。状态值大于等于2说明设备没有识别CWB命令是啥
		if (csw->return_value == 1) {
			set_error(node, KBURN_ERROR_KIND_COMMON, KBurnUsbErrorSense, "mass storage status: %d", csw->return_value);
		} else {
			set_error(node, KBURN_ERROR_KIND_COMMON, KBurnProtocolOpMismatch, "mass storage status: %d", csw->return_value);
		}
		return false;
	} else {
		debug_print(KBURN_LOG_TRACE, "mass storage status: %d", csw->return_value);
	}
	return true;
}

bool usb_lowlevel_status_read(kburnUsbDeviceNode *node, uint8_t endpoint_in, uint32_t expected_operation_index) {
	debug_trace_function("%d", expected_operation_index);

	usbIspResponsePacket csw;
	memset(&csw, 0, sizeof(usbIspResponsePacket));

	int readed_size = 0;
	ifNotReturnFalse(retry_libusb_bulk_transfer(node, endpoint_in, &csw, sizeof(usbIspResponsePacket), &readed_size, usb_timeout));

	print_buffer(KBURN_LOG_BUFFER, "csw", (void *)&csw, sizeof(usbIspResponsePacket));

	if (readed_size != sizeof(usbIspResponsePacket)) {
		debug_print(
			KBURN_LOG_ERROR, "usb_lowlevel_status_read: received %d bytes (expected " FMT_SIZET ")", readed_size, sizeof(usbIspResponsePacket));
	}

	if (!csw_status_parse(node, &csw, expected_operation_index)) {
		if (get_node(node)->error->code == make_error_code(KBURN_ERROR_KIND_COMMON, KBurnUsbReadIndexMismatch)) {
			return usb_lowlevel_status_read(node, endpoint_in, expected_operation_index);
		} else {
			return false;
		}
	}
	return true;
}

bool usb_lowlevel_transfer(kburnUsbDeviceNode *node, enum InOut direction, void *buffer, int size) {
	debug_trace_function("%s, %d bytes", direction == USB_READ ? "read" : "write", size);

	if (direction == USB_WRITE) {
		print_buffer(KBURN_LOG_BUFFER, "PC→USB", buffer, size);
	}

	int actual_size;
	uint8_t target = direction == USB_WRITE ? node->deviceInfo.endpoint_out : node->deviceInfo.endpoint_in;
	if (!retry_libusb_bulk_transfer(node, target, buffer, size, &actual_size, usb_timeout))
		return false;

	if (direction == USB_READ) {
		print_buffer(KBURN_LOG_BUFFER, "USB→PC", buffer, actual_size);
	}

	if (actual_size != size) {
		if (actual_size >= (int)sizeof(usbIspResponsePacket) && is_packet_type_csw(buffer)) {
			csw_status_parse(node, buffer, 0);
			return false;
		}

		_set_error(
			node->parent, KBURN_ERROR_KIND_COMMON, KBurnUsbSizeMismatch, "actual read/write size(%d) is not equal to expected (%d)", actual_size,
			size);
		return false;
	}
	return true;
}

/************************************************************
 * TODO 判断上一条指令出错的原因?
 *************************************************************/
bool usb_lowlevel_error_read(kburnUsbDeviceNode *node, uint8_t endpoint_in, uint8_t endpoint_out) {
	debug_trace_function();

	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_SENSE,
		.sense.size = REQUEST_SENSE_LENGTH,
	};

	uint8_t sense[REQUEST_SENSE_LENGTH];
	memset(sense, 0, REQUEST_SENSE_LENGTH);

	uint32_t expected_tag = rand();
	int size;

	// Request Sense

	if (!usb_lowlevel_command_send(node, endpoint_out, request, LIBUSB_ENDPOINT_IN, REQUEST_SENSE_LENGTH, expected_tag)) {
		return false;
	}

	ifNotReturnFalse(retry_libusb_bulk_transfer(node, endpoint_in, (unsigned char *)&sense, REQUEST_SENSE_LENGTH, &size, usb_timeout));

	print_buffer(KBURN_LOG_TRACE, "sense", sense, REQUEST_SENSE_LENGTH);

	if ((sense[0] != 0x70) && (sense[0] != 0x71)) {
		debug_print(KBURN_LOG_ERROR, "ERROR Sense: No data");
	}

	if (!usb_lowlevel_status_read(node, endpoint_in, expected_tag)) {
		return false;
	}
	return true;
}
