#pragma once

#include <libusb-1.0/libusb.h>

#define usb_interface interface
#define uchar2uint(buf) (((buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3])

#define VID 0x0559
#define PID 0x4001

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

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct usbdev_node
	{
		struct libusb_device_handle *handle; /* usb设备句柄 */
		uint8_t endpoint_in;				 /* 端点 in */
		uint8_t endpoint_out;				 /* 端点 out */
		unsigned char stringID[128];		 /* usb唯一ID */
		struct usbdev_node *next;
	} usbdev_node;

	/* libusb 初始化 */
	int libusbInit();

	/* 枚举设备并打开匹配到vid 和 pid 的设备 */
	int serchOpenDevice(uint16_t vid, uint16_t pid, usbdev_node *head);

	/* 从指定地址读取指定长度数据到缓冲区 */
	int readData(usbdev_node *usbdev, uint32_t length, uint64_t address, unsigned char *buffer, unsigned char *order);

	/* 从指定地址写入指定长度数据到缓冲区 */
	int writeData(usbdev_node *usbdev, uint32_t length, uint64_t address, unsigned char *buffer, unsigned char *order);

	/* 删除usb设备，当设备完成传输任务，执行这个 */
	int delUsbDev(usbdev_node *head, usbdev_node *usbdev);

	/* 获取usb设备的最大容量(字节数) */
	uint64_t getMemorySize(usbdev_node *usbdev, uint8_t dev_type);

	/* 控制LED灯 */
	int LEDCtrl(usbdev_node *usbdev, uint8_t *order);

	/* 询问设备是否准备好 */
	int isUsbReady(usbdev_node *usbdev);

	/* libusb退出，调用此函数后不能再用libusb接口 */
	void libusbExit();

#ifdef __cplusplus
}
#endif
