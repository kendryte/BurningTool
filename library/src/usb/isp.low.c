#include "usb.h"
#include "isp.h"

#define RETRY_MAX 5
#define REQUEST_SENSE_LENGTH 0x12

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
static_assert(sizeof(usbIspRequestPacket) == 31, "cbw packet must 31bytes");

typedef struct usbIspResponsePacket // CSW
{
	uint8_t signature[4];
	uint32_t operation_tag;
	uint32_t __reserved1;
	uint32_t is_success; // bool
} __attribute__((__packed__)) usbIspResponsePacket;
//  char (*__)[sizeof(usbIspResponsePacket)] = 1;
static_assert(sizeof(usbIspResponsePacket) == 16, "cbw packet must 16bytes");

static inline usbIspRequestPacket create_request(uint32_t operation_tag, uint32_t transfer_data_length, uint8_t flags, const usbIspCommandPacket body)
{
	usbIspRequestPacket r = {
		.signature = "USBC",
		.operation_tag = operation_tag,
		.transfer_data_length = transfer_data_length,
		.flags = flags,
		.__lun = 0,
		._body_length = 9,
		.body = body,
		.reserved = {0},
	};

	return r;
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
Return: 返回负数说明函数执行失败，返回0为成功
*************************************************************/
kburn_err_t usb_lowlevel_command_send(libusb_device_handle *handle, uint8_t endpoint,
									  const usbIspCommandPacket cdb, uint8_t direction, int data_length, uint32_t operation_index)
{
	debug_print("usb_lowlevel_command_send(%d)", operation_index);

	assert((handle != NULL) && "handle pointer is null!");
	assert((endpoint & LIBUSB_ENDPOINT_IN) == 0 && "cannot send command on IN endpoint"); /* 校验端点的传输方向 */

	usbIspRequestPacket cbw = create_request(operation_index, data_length, direction, cdb);

	int retry = 0;
	int r = 0, written_size = 0;
	do
	{
		// 传输长度必须正好是31个字节
		r = libusb_bulk_transfer(handle, endpoint, (unsigned char *)&cbw, sizeof(usbIspRequestPacket), &written_size, 1000);
		debug_print_libusb_error("usb_lowlevel_command_send", r);
		if (r == LIBUSB_ERROR_PIPE)
		{
			libusb_clear_halt(handle, endpoint);
		}
		retry++;
	} while ((r == LIBUSB_ERROR_PIPE) && (retry < RETRY_MAX));

	if (r < LIBUSB_SUCCESS)
	{
		return KBURN_ERROR_KIND_USB | r;
	}

	return KBurnNoErr;
}

/************************************************************
Function: usb_lowlevel_status_read
Description: 获取CSW响应指令,一共13个字节。
param1 : libusb_device_handle *handle          usb设备句柄
param2 : uint8_t endpoint                      端点I/O
param3 : expected_operation_index
Return: 返回负数说明函数执行失败，返回0为成功
*************************************************************/
kburn_err_t usb_lowlevel_status_read(libusb_device_handle *handle, uint8_t endpoint, uint32_t expected_operation_index)
{
	debug_print("usb_lowlevel_status_read(%d)", expected_operation_index);

	int i, r, size;
	usbIspResponsePacket csw;

	i = 0;
	do
	{
		r = libusb_bulk_transfer(handle, endpoint, (unsigned char *)&csw, sizeof(usbIspResponsePacket), &size, 1000);
		debug_print_libusb_error("usb_lowlevel_status_read", r);
		if (r == LIBUSB_ERROR_PIPE)
		{
			libusb_clear_halt(handle, endpoint);
		}
		i++;
	} while ((r == LIBUSB_ERROR_PIPE) && (i < RETRY_MAX));

	if (r < LIBUSB_SUCCESS)
	{
		return KBURN_ERROR_KIND_USB | r;
	}

	if (size != sizeof(usbIspResponsePacket))
	{
		debug_print("usb_lowlevel_status_read: received %d bytes (expected %ld)", size, sizeof(usbIspResponsePacket));
	}

	if (strncmp((void *)csw.signature, "USBS", 4))
	{
		debug_print("usb_lowlevel_status_read: mismatched signature: %.4s (expected USBS)", csw.signature);
		return KBURN_ERROR_KIND_COMMON | KBurnUsbReadIndexMismatch;
	}

	if (csw.operation_tag != expected_operation_index)
	{
		debug_print("usb_lowlevel_status_read: mismatched tags: %d (expected %d)", csw.operation_tag, expected_operation_index);
		return KBURN_ERROR_KIND_COMMON | KBurnUsbReadIndexMismatch;
	}

	debug_print("mass storage status: %X (%s)\n", csw.is_success, csw.is_success ? "FAILED" : "success");
	if (csw.is_success)
	{
		// 状态值为1说明有错误出现，应该使用GetSense获取错误原因。状态值大于等于2说明设备没有识别CWB命令是啥
		if (csw.is_success == 1)
			return KBURN_ERROR_KIND_COMMON | KBurnUsbErrorSense;
		else
			return KBURN_ERROR_KIND_COMMON | KBurnProtocolOpMismatch;
	}
	return KBurnNoErr;
}

/************************************************************
 * TODO
Function: usb_lowlevel_error_read
Description: 判断上一条指令出错的原因。
param1 : libusb_device_handle *handle          usb设备句柄
param2 : uint8_t endpoint_in                   端点I/O
param3 : uint8_t endpoint_out                  端点I/O
*************************************************************/
kburn_err_t usb_lowlevel_error_read(libusb_device_handle *handle, uint8_t endpoint_in, uint8_t endpoint_out)
{
	usbIspCommandPacket request = {
		.command = USB_ISP_COMMAND_SENSE,
		.sense.size = REQUEST_SENSE_LENGTH,
	};

	uint8_t sense[REQUEST_SENSE_LENGTH];
	memset(sense, 0, REQUEST_SENSE_LENGTH);

	uint32_t expected_tag = rand();
	int size;
	int rc;

	// Request Sense
	debug_print("usb_lowlevel_error_read:\n");

	rc = usb_lowlevel_command_send(handle, endpoint_out, request, LIBUSB_ENDPOINT_IN, REQUEST_SENSE_LENGTH, expected_tag);
	if (rc < LIBUSB_SUCCESS)
	{
		return KBURN_ERROR_KIND_USB | rc;
	}
	rc = libusb_bulk_transfer(handle, endpoint_in, (unsigned char *)&sense, REQUEST_SENSE_LENGTH, &size, 1000);
	if (rc < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("libusb_bulk_transfer failed:", rc);
		return KBURN_ERROR_KIND_USB | rc;
	}
	debug_print("received %d bytes\n", size);

	if ((sense[0] != 0x70) && (sense[0] != 0x71))
	{
		debug_print("ERROR Sense: No data\n");
	}
	else
	{
		print_buffer("ERROR SENSE", sense, REQUEST_SENSE_LENGTH);
	}

	usb_lowlevel_status_read(handle, endpoint_in, expected_tag);
	return KBurnNoErr;
}
