#include <libusb-1.0/libusb.h>

#define usb_interface interface
#define uchar2uint(buf) (((buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3])

#define DEFAULT_VID 0x0559
#define DEFAULT_PID 0x4001

#define BLOCK_SIZE 512			   /* 一个块的大小 */
#define DATA_SIZE 1024 * 1024 * 10 /* 传输一次数据量的大小 */

#define READ_CAPACITY_LENGTH 0x08

// Mass Storage Requests values. See section 3 of the Bulk-Only Mass Storage Class specifications
#define BOMS_RESET 0xFF
#define BOMS_GET_MAX_LUN 0xFE

typedef enum usbIspCommand
{
	USB_ISP_COMMAND_HELLO = 0x0000,
	USB_ISP_COMMAND_SENSE = 0x0300,
	USB_ISP_COMMAND_WRITE_BURN = 0xE000,
	USB_ISP_COMMAND_WRITE_DEVICE = 0xE002,
	USB_ISP_COMMAND_READ_BURN = 0xE100,
	USB_ISP_COMMAND_READ_CAPACITY = 0xE101,
	USB_ISP_COMMAND_MAX = UINT16_MAX,
} __attribute__((__packed__)) usbIspCommand;
static_assert(sizeof(usbIspCommand) == 2, "enum must 16bit");

typedef enum usbIspCommandTaget
{
	USB_ISP_COMMAND_TARGET_EMMC = 0x00,
	USB_ISP_COMMAND_TARGET_SDCARD = 0x01,
	USB_ISP_COMMAND_TARGET_NAND = 0x02,
	USB_ISP_COMMAND_TARGET_OTP = 0x03,
	USB_ISP_COMMAND_TARGET_LED = 0x04,
	USB_ISP_COMMAND_TARGET_UART = 0x05,

	MAX_USB_ISP_COMMAND = UINT8_MAX,
} __attribute__((__packed__)) usbIspCommandTaget;
static_assert(sizeof(usbIspCommandTaget) == 1, "enum must 8bit");

struct usbIspCommandPacketBurnBody
{
	uint32_t address;
	uint16_t block_count;
} __attribute__((__packed__));
struct usbIspCommandPacketLedBody
{
	uint8_t led_io;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} __attribute__((__packed__));

typedef struct usbIspCommandPacket
{
	usbIspCommand command;
	usbIspCommandTaget target;
	union
	{
		struct usbIspCommandPacketBurnBody burn;
		struct usbIspCommandPacketLedBody led;
		uint32_t uart;
		struct
		{
			uint8_t _reserved;
			uint8_t size;
		} __attribute__((__packed__)) sense;
		uint8_t empty[6];
	} __attribute__((__packed__));
} __attribute__((__packed__)) usbIspCommandPacket;
// char (*__)[sizeof(usbIspCommandPacket)] = 1; // test
static_assert(sizeof(usbIspCommandPacket) == 9, "cwd packet must 9bytes");
