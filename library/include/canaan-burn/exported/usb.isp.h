#pragma once

#include "./prefix.h"
#include "./types.h"

DEFINE_START

#define KBURN_BOARD_PIN_LED_K510_CRB_V12B2 97

typedef enum kburnUsbIspCommandTaget {
	KBURN_USB_ISP_EMMC = 0x00,
	KBURN_USB_ISP_SDCARD = 0x01,
	KBURN_USB_ISP_NAND = 0x02,
	KBURN_USB_ISP_OTP = 0x03,
} kburnUsbIspCommandTaget;

struct kburnColor {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

static inline struct kburnColor kburnConvertColor(uint32_t color) {
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

typedef struct kburnDeviceMemorySizeInfo {
	kburnUsbIspCommandTaget device;
	uint32_t block_size;					 /* 一个块的字节数（通常为512） */
	uint64_t storage_size;					 /* 设备的总容量（字节） */
	kburn_stor_address_t base_address;		 /* 设备起始地址，目前均为0 */
	kburn_stor_block_t block_count;			 /* 设备的总容量（块） */
	kburn_stor_address_t last_block_address; /* 最后一个块的地址 */
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
 * @param address 读取地址，**以块数为单位，基地址=0**
 * @param length 数据长度，单位字节，函数处理时会转成块，所以传入字节需要是dev_info.block_size的整数倍，一次最多10MB数据
 * @param buffer 输出缓冲区，长度必须至少为length
 * @param dev_info 设备块大小信息
 */
kburn_err_t kburnUsbIspReadChunk(kburnDeviceNode *node, const kburnDeviceMemorySizeInfo dev_info, kburn_stor_block_t address, uint32_t length,
								 void *buffer);

/**
 * 将buffer写入target的address地址
 * @param target 写入目标设备
 * @param address 写入地址，**以块数为单位，基地址=0**
 * @param buffer 数据缓冲区
 * @param buffer_size 数据长度，单位字节，函数处理时会转成块，所以传入字节需要是dev_info.block_size的整数倍，一次最多10MB数据
 * @param dev_info 设备块大小信息
 */
kburn_err_t kburnUsbIspWriteChunk(kburnDeviceNode *node, const kburnDeviceMemorySizeInfo dev_info, kburn_stor_block_t address, void *buffer,
								  uint32_t length);
/** 写入数据，但不要求任何对齐，效率较低，注意地址单位是字节 */
kburn_err_t kburnUsbIspReadUnaligned(kburnDeviceNode *node, const kburnDeviceMemorySizeInfo dev_info, kburn_stor_address_t address, uint32_t length,
									 void *buffer);
/** 读取数据，但不要求任何对齐，效率较低，注意地址单位是字节 */
kburn_err_t kburnUsbIspWriteUnaligned(kburnDeviceNode *node, const kburnDeviceMemorySizeInfo dev_info, kburn_stor_address_t address, void *buffer,
									  uint32_t length);

DEFINE_END
