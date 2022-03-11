#include <libusb-1.0/libusb.h>

#define usb_interface interface
#define uchar2uint(buf) (((buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3])

#define DEFAULT_VID 0x0559
#define DEFAULT_PID 0x4001

#define BLOCK_SIZE 512			   /* 一个块的大小 */
#define DATA_SIZE 1024 * 1024 * 10 /* 传输一次数据量的大小 */

#define RETRY_MAX 5
#define REQUEST_SENSE_LENGTH 0x12
#define READ_CAPACITY_LENGTH 0x08

#define WR_CMD 0XE0
#define READ_CMD 0XE1
#define READY_CMD 0X00

#define LED_DEV 0X04

// Mass Storage Requests values. See section 3 of the Bulk-Only Mass Storage Class specifications
#define BOMS_RESET 0xFF
#define BOMS_GET_MAX_LUN 0xFE

typedef struct usbdev_node
{
	struct libusb_device_handle *handle; /* usb设备句柄 */
	uint8_t endpoint_in;				 /* 端点 in */
	uint8_t endpoint_out;				 /* 端点 out */
	unsigned char stringID[128];		 /* usb唯一ID */
	struct usbdev_node *next;
} usbdev_node;
