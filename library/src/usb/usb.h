#pragma once

#include <stdlib.h>
#include "global.h"
#include "./usb.types.h"

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

typedef struct usb_subsystem_context
{
	struct
	{
		int vid;
		int pid;
	} filter;

	struct libusb_context *libusb;
	bool inited;
	bool monitor_enabled;
} usb_subsystem_context;

kburn_err_t usb_subsystem_init(KBCTX scope);
void usb_subsystem_deinit(KBCTX scope);
void usb_monitor_pause(KBCTX scope);
kburn_err_t usb_monitor_resume(KBCTX scope);
kburn_err_t init_list_usb(KBCTX scope);

void destroy_usb_port(KBCTX scope, kburnDeviceNode *device);
kburn_err_t create_usb_port(KBCTX scope, kburnDeviceNode *device);
