#pragma once

#include "canaan-burn/canaan-burn.h"
#include "isp.h"
#include <libusb.h>

typedef enum usbIspCommand {
	USB_ISP_COMMAND_HELLO = 0x0000,
	USB_ISP_COMMAND_SENSE = 0x0300,
	USB_ISP_COMMAND_WRITE_BURN = 0xE000,
	USB_ISP_COMMAND_WRITE_DEVICE = 0xE002,
	USB_ISP_COMMAND_READ_BURN = 0xE100,
	USB_ISP_COMMAND_READ_CAPACITY = 0xE101,
	USB_ISP_COMMAND_MAX = UINT16_MAX,
} __attribute__((__packed__)) usbIspCommand;
_Static_assert(sizeof(usbIspCommand) == 2, "enum must 16bit");

typedef enum usbIspCommandTaget {
	USB_ISP_COMMAND_TARGET_EMMC = KBURN_USB_ISP_EMMC,
	USB_ISP_COMMAND_TARGET_SDCARD = KBURN_USB_ISP_SDCARD,
	USB_ISP_COMMAND_TARGET_NAND = KBURN_USB_ISP_NAND,
	USB_ISP_COMMAND_TARGET_OTP = KBURN_USB_ISP_OTP,
	USB_ISP_COMMAND_TARGET_LED = 0x04,
	USB_ISP_COMMAND_TARGET_UART = 0x05,

	MAX_USB_ISP_COMMAND = UINT8_MAX,
} __attribute__((__packed__)) usbIspCommandTaget;
_Static_assert(sizeof(usbIspCommandTaget) == 1, "enum must 8bit");

struct usbIspCommandPacketBurnBody {
	kburn_stor_address_t address;
	uint16_t block_count;
} __attribute__((__packed__));
struct usbIspCommandPacketLedBody {
	uint8_t led_io;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} __attribute__((__packed__));

typedef struct usbIspCommandPacket {
	usbIspCommand command;
	usbIspCommandTaget target;
	union {
		struct usbIspCommandPacketBurnBody burn;
		struct usbIspCommandPacketLedBody led;
		uint8_t uart[6];
		struct {
			uint8_t _reserved;
			uint8_t size;
		} __attribute__((__packed__)) sense;
		uint8_t empty[6];
	} __attribute__((__packed__));
} __attribute__((__packed__)) usbIspCommandPacket;
// char (*__)[sizeof(usbIspCommandPacket)] = 1; // test
_Static_assert(sizeof(usbIspCommandPacket) == 9, "cwd packet must 9bytes");

enum InOut {
	USB_READ,
	USB_WRITE,
};

kburn_err_t usb_lowlevel_command_send(libusb_device_handle *handle, uint8_t endpoint, const usbIspCommandPacket cdb, uint8_t direction,
									  int data_length, uint32_t operation_index);
kburn_err_t usb_lowlevel_status_read(libusb_device_handle *handle, uint8_t endpoint, uint32_t expected_operation_index);
kburn_err_t usb_lowlevel_error_read(libusb_device_handle *handle, uint8_t endpoint_in, uint8_t endpoint_out);
kburn_err_t usb_lowlevel_transfer(kburnUsbDeviceNode *node, enum InOut direction, void *buffer, int size);
