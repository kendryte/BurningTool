#pragma once

#include "./prefix.h"

DEFINE_START

#define KBURN_BOARD_PIN_LED_K510_CRB_V12B2 97

typedef enum kburnUsbIspCommandTaget
{
	KBURN_USB_ISP_EMMC = 0x00,
	KBURN_USB_ISP_SDCARD = 0x01,
	KBURN_USB_ISP_NAND = 0x02,
	KBURN_USB_ISP_OTP = 0x03,
} kburnUsbIspCommandTaget;

struct kburnColor
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

static inline struct kburnColor kburnConvertColor(uint32_t color)
{
	return (struct kburnColor){
		.red = (uint8_t)(color >> 16),
		.green = (uint8_t)(color >> 8),
		.blue = (uint8_t)(color),
	};
}

/**
 *
 */
kburn_err_t kburnUsbIspLedControl(kburnDeviceNode *node, uint8_t pin, struct kburnColor color);

typedef struct kburnDeviceMemorySizeInfo
{
	uint32_t block_size;	 /* 一个块的字节数,通常为512 */
	uint64_t device_size;	 /* usb设备的总容量，单位：字节 */
	uint32_t max_block_addr; /* 最大的块地址，从0开始数 */
} kburnDeviceMemorySizeInfo;
/**
 * 获取块设备大小信息
 * @param target 设备
 * @param out_dev_info 输出参数
 */
kburn_err_t kburnUsbIspGetMemorySize(kburnDeviceNode *node, kburnUsbIspCommandTaget target, kburnDeviceMemorySizeInfo *out_dev_info);

/**
 * 从target的address地址读取length长度数据到buffer
 * @param target 读取目标设备
 * @param address 读取地址，将会转成块地址，也就是第几个block
 * @param length 数据长度，单位字节，函数处理时会转成块，所以传入字节需要是dev_info.block_size的整数倍
 * @param buffer 输出缓冲区，长度必须至少为length
 * @param dev_info 设备块大小信息
 */
kburn_err_t kburnUsbIspReadData(kburnDeviceNode *node, kburnUsbIspCommandTaget target, uint64_t address, uint32_t length, unsigned char *buffer, const kburnDeviceMemorySizeInfo *dev_info);

/****************************************************************************
Function: writeData
Description: 从指定地址写入指定长度数据到缓冲区。
param1 : usbdev_node* usbdev          usb节点句柄
param2 : uint32_t length              数据长度(单位：字节，函数处理时会转成块)
										  所以传入字节需要是512字节的整数倍。
param3 : uint32_t address             写入地址(将会转成块地址，也就是第几个block)
param4 : unsigned char *buffer        缓冲区
param5 : order                        烧录、读取容量，存储器类型
Return: 返回负数说明函数执行失败，返回0为成功
******************************************************************************/

/**
 * 将buffer写入target的address地址
 * @param target 写入目标设备
 * @param address 写入地址，将会转成块地址，也就是第几个block
 * @param buffer 数据缓冲区
 * @param buffer_size 数据长度，单位字节，函数处理时会转成块，所以传入字节需要是dev_info.block_size的整数倍
 * @param dev_info 设备块大小信息
 */
kburn_err_t kburnUsbIspWriteData(kburnDeviceNode *node, kburnUsbIspCommandTaget target, uint64_t address, unsigned char *buffer, uint32_t length, const kburnDeviceMemorySizeInfo *dev_info);

DEFINE_END
