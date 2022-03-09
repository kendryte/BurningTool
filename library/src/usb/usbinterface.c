#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "usbinterface.h"

// Command Block Wrapper (CBW)
struct command_block_wrapper
{
	uint8_t dCBWSignature[4];
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
};

// Command Status Wrapper (CSW)
struct command_status_wrapper
{
	uint8_t dCSWSignature[4];
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
};

/*************************************************
Function: libusbInit()
Description: libusb 初始化
Return: 函数返回值为负数，说明初始化失败，返回0成功
*************************************************/
int libusbInit()
{
	int r;
	const struct libusb_version *version;
	/* libusb 的版本 */
	version = libusb_get_version();
	printf("Using libusb v%d.%d.%d.%d\n\n", version->major, version->minor, version->micro, version->nano);
	/* libusb 初始化 */
	r = libusb_init(NULL);
	if (r < 0)
	{
		printf("Libusb init failed: %s\n", libusb_strerror((enum libusb_error)r));
		return r;
	}
	/* 设置日志级别 */
	r = libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
	if (r < 0)
	{
		printf("Log level set failed: %s\n", libusb_strerror((enum libusb_error)r));
	}
	return 0;
}

/***************************************************
Function: libusbExit()
Description: libusb退出，调用此函数后不能再用libusb接口
****************************************************/
void libusbExit()
{
	libusb_exit(NULL);
}

/****************************************************
Function: getDevEndpoint
Description: 获取usb设备的端点。in和out
Called By:   由函数serchOpenDevice()调用
param1 : dev         枚举出来并符合vid pid的usb设备
param2 : current     当前设备节点
Return: 返回负数说明获取端点失败，返回0为成功
*******************************************************/
static int getDevEndpoint(libusb_device *dev, usbdev_node *current)
{
	int r;
	int k, j;
	struct libusb_config_descriptor *conf_desc;
	const struct libusb_endpoint_descriptor *endpoint;

	/* 获取配置描述符，里面包含端点信息 */
	r = libusb_get_config_descriptor(dev, 0, &conf_desc);
	if (r < 0)
	{
		printf("Get config descriptor failed: %s\n", libusb_strerror((enum libusb_error)r));
		return -1;
	}

	/* 默认使用第一个接口 usb_interface[0]*/
	for (j = 0; j < conf_desc->usb_interface[0].num_altsetting; j++)
	{
		for (k = 0; k < conf_desc->usb_interface[0].altsetting[j].bNumEndpoints; k++)
		{
			endpoint = &conf_desc->usb_interface[0].altsetting[j].endpoint[k];
			printf("Endpoint[%d].address: %02X\n", k, endpoint->bEndpointAddress);
			/* 使用批量传输的端点 */
			if (endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK & LIBUSB_TRANSFER_TYPE_BULK)
			{
				if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN)
				{
					current->endpoint_in = endpoint->bEndpointAddress;
				}
				else
				{
					current->endpoint_out = endpoint->bEndpointAddress;
				}
			}
		}
	}
	/* 释放配置描述符 */
	libusb_free_config_descriptor(conf_desc);
	return 0;
}

/************************************************************
Function: getmaxlun
Description: 获取usb设备最大的lun
Return: 返回lun的最大值，如果不支持lun,返回0
Other: 目前所看到的设备都只有一个lun,所以此函数目前不用
*************************************************************/
__attribute__((unused)) static uint8_t getmaxlun(libusb_device_handle *dev)
{
	int r;
	uint8_t lun = 0;
	printf("Reading Max LUN:\n");
	r = libusb_control_transfer(dev, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
								BOMS_GET_MAX_LUN, 0, 0, &lun, 1, 1000);
	// Some devices send a STALL instead of the actual value.
	// In such cases we should set lun to 0.
	if (r == 0)
	{
		lun = 0;
	}

	else if (r < 0)
	{
		printf("Failed: %s", libusb_strerror((enum libusb_error)r));
	}
	printf("Max LUN = %d\n", lun);
	return lun;
}

/************************************************************
Function: serchOpenDevice
Description: 枚举设备并打开匹配到vid 和 pid 的设备。将打开的usb设备
			存储到链表中。每找到一个符合要求的设备，便建立一个节点
param1 : uint16_t vid            产品的vid
param2 : uint16_t pid            产品的pid
param3 : usbdev_node *head       头节点
Return: 返回负数说明函数执行失败，返回0为成功
*************************************************************/
int serchOpenDevice(uint16_t vid, uint16_t pid, usbdev_node *head)
{
	if (head == NULL)
	{
		printf("There is no head node\n");
		return -1;
	}

	int find_dev_flag = -1;
	ssize_t cnt = 0;
	int m = 0;
	unsigned char string[128];
	memset(string, 0, sizeof(string));
	libusb_device **devs;
	libusb_device *dev;
	usbdev_node *current = head;
	usbdev_node *tail = head;

	/* 枚举设备 */
	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
	{
		printf("Get device list failed: %s\n", libusb_strerror((enum libusb_error)cnt));
		libusb_exit(NULL);
		return -1;
	}

	while ((dev = devs[m++]) != NULL)
	{
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);

		if (r < 0)
		{
			printf("Failed to get device descriptor: %s\n", libusb_strerror((enum libusb_error)r));
			return -1;
		}

		if (desc.idVendor == vid && desc.idProduct == pid)
		{
			/* 如果匹配到设备就建立设备节点 */
			find_dev_flag = 1;
			current = (usbdev_node *)malloc(sizeof(usbdev_node));
			memset(string, 0, sizeof(string));
			r = libusb_open(dev, &current->handle);
			if (r < 0)
			{
				printf("Open device failed: %s\n", libusb_strerror((enum libusb_error)r));
				continue;
			}

			if (libusb_get_string_descriptor_ascii(current->handle, desc.iSerialNumber, (unsigned char *)string, sizeof(string)) > 0)
			{
				printf("\nString (0x%02X): \"%s\"\n", desc.iSerialNumber, string);
				strcpy((char *)current->stringID, (char *)string);
			}

			r = getDevEndpoint(dev, current);
			if (r < 0)
			{
				printf("Get endpoint failed");
			}

			/* 声明接口,记得所有数据传输完成之后要释放*/
			r = libusb_claim_interface(current->handle, 0);
			if (r < 0)
			{
				printf("Claiming interface Failed.%s\n", libusb_strerror((enum libusb_error)r));
			}

			tail->next = current;
			current->next = NULL;
			tail = current;
		}
	}
	libusb_free_device_list(devs, 1);
	return find_dev_flag;
}

/************************************************************
Function: sendCWBCommand
Description: 发送CWB指令,一共31个字节。
param1 : libusb_device_handle *handle          usb设备句柄
param2 : uint8_t endpoint                      端点I/O
param3 : uint8_t lun                           lun
param4 : uint8_t *cdb                          CDB字段
param5 : uint8_t direction                     传输方向
param6 : int data_length                       数据长度（字节）
param7 : uint32_t *ret_tag                     程序执行成功与否
Return: 返回负数说明函数执行失败，返回0为成功
*************************************************************/
static int sendCWBCommand(libusb_device_handle *handle, uint8_t endpoint, uint8_t lun,
						  uint8_t *cdb, uint8_t direction, int data_length, uint32_t *ret_tag)
{
	static uint32_t tag = 1;
	uint8_t cdb_len = 9;
	int i, r, size;
	struct command_block_wrapper cbw;

	if (cdb == NULL)
	{
		return -1;
	}

	/* 校验端点的传输方向 */
	if (endpoint & LIBUSB_ENDPOINT_IN)
	{
		printf("sendCWBCommand: cannot send command on IN endpoint\n");
		return -1;
	}

	/* 填充CBW结构 */
	memset(&cbw, 0, sizeof(cbw));
	cbw.dCBWSignature[0] = 'U';
	cbw.dCBWSignature[1] = 'S';
	cbw.dCBWSignature[2] = 'B';
	cbw.dCBWSignature[3] = 'C';
	*ret_tag = tag;
	cbw.dCBWTag = tag++;
	cbw.dCBWDataTransferLength = data_length;
	cbw.bmCBWFlags = direction;
	cbw.bCBWLUN = lun;
	cbw.bCBWCBLength = cdb_len;
	memcpy(cbw.CBWCB, cdb, cdb_len);

	i = 0;
	do
	{
		//传输长度必须正好是31个字节
		r = libusb_bulk_transfer(handle, endpoint, (unsigned char *)&cbw, 31, &size, 1000);
		if (r == LIBUSB_ERROR_PIPE)
		{
			libusb_clear_halt(handle, endpoint);
		}
		i++;
	} while ((r == LIBUSB_ERROR_PIPE) && (i < RETRY_MAX));

	if (r != LIBUSB_SUCCESS)
	{
		printf("   sendCWBCommand: %s\n", libusb_strerror((enum libusb_error)r));
		return -1;
	}

	// printf("   sent %d CDB bytes\n", cdb_len);
	return 0;
}

/************************************************************
Function: getCswReply
Description: 获取CSW响应指令,一共13个字节。
param1 : libusb_device_handle *handle          usb设备句柄
param2 : uint8_t endpoint                      端点I/O
param3 : uint32_t expected_tag                 程序执行成功与否错误码
Return: 返回负数说明函数执行失败，返回0为成功
*************************************************************/
static int getCswReply(libusb_device_handle *handle, uint8_t endpoint, uint32_t expected_tag)
{
	int i, r, size;
	struct command_status_wrapper csw;

	// The device is allowed to STALL this transfer. If it does, you have to
	// clear the stall and try again.
	i = 0;
	do
	{
		r = libusb_bulk_transfer(handle, endpoint, (unsigned char *)&csw, 13, &size, 1000);
		if (r == LIBUSB_ERROR_PIPE)
		{
			libusb_clear_halt(handle, endpoint);
		}
		i++;
	} while ((r == LIBUSB_ERROR_PIPE) && (i < RETRY_MAX));

	if (r != LIBUSB_SUCCESS)
	{
		printf("   getCswReply: %s\n", libusb_strerror((enum libusb_error)r));
		return -1;
	}

	if (size != 13)
	{
		printf("   getCswReply: received %d bytes (expected 13)\n", size);
		return -1;
	}

	if (csw.dCSWTag != expected_tag)
	{
		printf("   getCswReply: mismatched tags (expected %08X, received %08X)\n",
			   expected_tag, csw.dCSWTag);
		return -1;
	}
	/* 没有检查 0~3 位的合法性 " 'U' 'S' 'B' 'C'" */
	printf("   Mass Storage Status: %02X (%s)\n", csw.bCSWStatus, csw.bCSWStatus ? "FAILED" : "Success");
	if (csw.dCSWTag != expected_tag)
		return -1;
	if (csw.bCSWStatus)
	{
		//状态值为1说明有错误出现，使用GetSense获取错误原因。状态值大于等于2说明设备没有识别CWB命令是啥
		if (csw.bCSWStatus == 1)
			return -2;
		else
			return -1;
	}
	return 0;
}

/************************************************************
Function: getSense
Description: 判断上一条指令出错的原因。
param1 : libusb_device_handle *handle          usb设备句柄
param2 : uint8_t endpoint_in                   端点I/O
param3 : uint8_t endpoint_out                  端点I/O
*************************************************************/
static void getSense(libusb_device_handle *handle, uint8_t endpoint_in, uint8_t endpoint_out)
{
	uint8_t cdb[16]; // SCSI Command Descriptor Block
	uint8_t sense[18];
	uint32_t expected_tag;
	int size;
	int rc;

	// Request Sense
	printf("Request Sense:\n");
	memset(sense, 0, sizeof(sense));
	memset(cdb, 0, sizeof(cdb));
	cdb[0] = 0x03; // Request Sense
	cdb[4] = REQUEST_SENSE_LENGTH;

	sendCWBCommand(handle, endpoint_out, 0, cdb, LIBUSB_ENDPOINT_IN, REQUEST_SENSE_LENGTH, &expected_tag);
	rc = libusb_bulk_transfer(handle, endpoint_in, (unsigned char *)&sense, REQUEST_SENSE_LENGTH, &size, 1000);
	if (rc < 0)
	{
		printf("libusb_bulk_transfer failed: %s\n", libusb_error_name(rc));
		return;
	}
	printf("received %d bytes\n", size);

	if ((sense[0] != 0x70) && (sense[0] != 0x71))
	{
		printf("ERROR No sense data\n");
	}
	else
	{
		printf("ERROR Sense: %02X %02X %02X\n", sense[2] & 0x0F, sense[12], sense[13]);
	}
	getCswReply(handle, endpoint_in, expected_tag);
}

/*****************************************************************
Function: relaseClaimedInterfaces
Description: 释放所有声明过的接口端点，需要在所有数据传输完成之后调用
param1 :  head            设备列表的头节点
Return 返回负数说明函数执行失败，返回0为成功
******************************************************************/
int relaseClaimedInterfaces(usbdev_node *head)
{
	int r;
	usbdev_node *usbdev = head->next;
	while (usbdev)
	{
		r = libusb_release_interface(usbdev->handle, 0);
		if (r < 0)
		{
			printf("relase interface failed:%s\n", libusb_strerror((enum libusb_error)r));
		}
		usbdev = usbdev->next;
	}
	return r;
}

/****************************************************************************
Function: readData
Description: 从指定地址读取指定长度数据到缓冲区。
param1 : usbdev          usb节点句柄
param2 : length          数据长度(单位：字节，函数处理时会转成块)。
								所以传入字节需要是512字节的整数倍。
param3 : address         读取地址(将会转成块地址，也就是第几个block)
param4 : buffer          缓冲区
param5 : order           其他命令，存储器类型，点灯命令
Return: 返回负数说明函数执行失败，返回0为成功
*****************************************************************************/
int readData(usbdev_node *usbdev, uint32_t length, uint64_t address, unsigned char *buffer, unsigned char *order)
{
	if (buffer == NULL || order == NULL)
	{
		printf("Pointer is invalid\n");
		return -1;
	}
	int r;
	uint8_t cdb[16]; /* SCSI 命令 */
	uint8_t lun = 0;
	uint16_t block_num = length % BLOCK_SIZE == 0 ? length / BLOCK_SIZE : length / BLOCK_SIZE + 1; /* 传输长度是以block的个数计数的，一个block是512字节 */
	uint32_t block_addr = (uint32_t)(address / BLOCK_SIZE);
	uint32_t expected_tag;
	uint8_t *u8_adr = (uint8_t *)&block_addr;
	uint8_t *u8_len = (uint8_t *)&block_num;
	int actual_size;

	memset(cdb, 0, sizeof(cdb));

	cdb[0] = READ_CMD;	   /* read cmd */
	cdb[1] = *order;	   /* 操作类型 */
	cdb[2] = *(order + 1); /* 设备类型 */

	cdb[3] = *(u8_adr + 3); /* 地址 uint32 转 uint8 */
	cdb[4] = *(u8_adr + 2);
	cdb[5] = *(u8_adr + 1);
	cdb[6] = *(u8_adr + 0);

	cdb[7] = *(u8_len + 1); /* 长度：block的个数 */
	cdb[8] = *(u8_len + 0);

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = sendCWBCommand(usbdev->handle, usbdev->endpoint_out, lun, cdb, LIBUSB_ENDPOINT_IN, block_num * BLOCK_SIZE, &expected_tag);
	if (r < 0)
	{
		printf("Send CWB command failed\n");
		return -1;
	}
	/* 数据阶段 */
	r = libusb_bulk_transfer(usbdev->handle, usbdev->endpoint_in, buffer, block_num * BLOCK_SIZE, &actual_size, 5000);

	if (actual_size != block_num * BLOCK_SIZE)
	{
		printf("actual read size(%d) is not equal expect read size(%d)\n", actual_size, block_num * BLOCK_SIZE);
	}
	/* 状态阶段 */
	if (getCswReply(usbdev->handle, usbdev->endpoint_in, expected_tag) == -2)
	{
		getSense(usbdev->handle, usbdev->endpoint_in, usbdev->endpoint_out);
	}
	return 0;
}

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
int writeData(usbdev_node *usbdev, uint32_t length, uint64_t address, unsigned char *buffer, unsigned char *order)
{
	if (buffer == NULL || order == NULL)
	{
		printf("Pointer is invalid\n");
		return -1;
	}
	int r;
	uint8_t cdb[16]; /* SCSI 命令 */
	uint8_t lun = 0;
	uint16_t block_num = length % BLOCK_SIZE == 0 ? length / BLOCK_SIZE : length / BLOCK_SIZE + 1; /* 传输长度是以block的个数计数的，一个block是512字节 */
	uint32_t block_addr = (uint32_t)(address / BLOCK_SIZE);										   /* 地址是以block计算的 */
	uint32_t expected_tag;
	uint8_t *u8_adr = (uint8_t *)&block_addr;
	uint8_t *u8_len = (uint8_t *)&block_num;
	int actual_size;
	memset(cdb, 0, sizeof(cdb));

	cdb[0] = WR_CMD;	   /* 写数据 */
	cdb[1] = *order;	   /* 操作类型 */
	cdb[2] = *(order + 1); /* 设备类型 */

	cdb[3] = *(u8_adr + 3); /* 地址 uint32 转 uint8 */
	cdb[4] = *(u8_adr + 2);
	cdb[5] = *(u8_adr + 1);
	cdb[6] = *(u8_adr + 0);

	cdb[7] = *(u8_len + 1); /* 长度：block的个数 */
	cdb[8] = *(u8_len + 0);

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = sendCWBCommand(usbdev->handle, usbdev->endpoint_out, lun, cdb, LIBUSB_ENDPOINT_OUT, block_num * BLOCK_SIZE, &expected_tag);
	if (r < 0)
	{
		printf("Send CWB command failed\n");
		return -1;
	}
	/* 数据阶段 */
	r = libusb_bulk_transfer(usbdev->handle, usbdev->endpoint_out, buffer, block_num * BLOCK_SIZE, &actual_size, 5000);
	if (r < 0)
	{
		printf("libusb_bulk_transfer failed: %s\n", libusb_error_name(r));
	}
	if (actual_size != block_num * BLOCK_SIZE)
	{
		printf("Actual write size(%d) is not equal to expect write size(%d)\n", actual_size, block_num * BLOCK_SIZE);
	}
	/* 状态阶段 */
	if (getCswReply(usbdev->handle, usbdev->endpoint_in, expected_tag) == -2)
	{
		getSense(usbdev->handle, usbdev->endpoint_in, usbdev->endpoint_out);
	}
	return 0;
}

/****************************************************************************
Function: LEDCtrl（）
Description: 控制LED灯亮。
param1 : usbdev_node* usbdev          usb节点句柄
param2 : uint8_t *order               LED控制命令 IO/R/G/B
Return: 返回0说明成功，返回-1失败
******************************************************************************/
int LEDCtrl(usbdev_node *usbdev, uint8_t *order)
{
	if (order == NULL)
	{
		printf("LEDCtrl Pointer is invalid\n");
		return -1;
	}
	int r;
	uint8_t cdb[16]; /* SCSI 命令 */
	uint8_t lun = 0;
	uint32_t expected_tag;
	memset(cdb, 0, sizeof(cdb));

	cdb[0] = WR_CMD; /* 写数据 */
	cdb[1] = 0x02;
	cdb[2] = LED_DEV;
	cdb[3] = *order;
	cdb[4] = *(order + 1);
	cdb[5] = *(order + 2);
	cdb[6] = *(order + 3);

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = sendCWBCommand(usbdev->handle, usbdev->endpoint_out, lun, cdb, LIBUSB_ENDPOINT_OUT, 0, &expected_tag);
	if (r < 0)
	{
		printf("Send CWB command failed\n");
		return -1;
	}
	/* 数据阶段 */

	/* 没有数据阶段了*/

	/* 状态阶段 */
	if (getCswReply(usbdev->handle, usbdev->endpoint_in, expected_tag) == -2)
	{
		getSense(usbdev->handle, usbdev->endpoint_in, usbdev->endpoint_out);
	}
	return 0;
}
/****************************************************************************
Function: getMemorySize（）
Description: 获取USB设备的最大容量。
param1 : usbdev_node* usbdev          usb节点句柄
Return: 返回USB设备容量的总字节数。
******************************************************************************/
uint64_t getMemorySize(usbdev_node *usbdev, uint8_t dev_type)
{
	int r;
	uint8_t cdb[16]; /* SCSI 命令 */
	uint8_t lun = 0;
	uint8_t buffer[64];
	uint32_t expected_tag;
	int actual_size;

	uint32_t max_block_addr = 0;
	uint32_t block_size = 0;
	uint64_t device_size = 0;
	memset(cdb, 0, sizeof(cdb));

	cdb[0] = READ_CMD; // READ CAPACITY
	cdb[1] = 0x01;
	cdb[2] = dev_type;

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = sendCWBCommand(usbdev->handle, usbdev->endpoint_out, lun, cdb, LIBUSB_ENDPOINT_IN, READ_CAPACITY_LENGTH, &expected_tag);
	if (r < 0)
	{
		printf("Send CWB command failed\n");
		return -1;
	}
	/* 数据阶段 */
	r = libusb_bulk_transfer(usbdev->handle, usbdev->endpoint_in, buffer, READ_CAPACITY_LENGTH, &actual_size, 1000);
	if (r < 0)
	{
		printf("libusb_bulk_transfer failed: %s\n", libusb_error_name(r));
	}
	if (actual_size != READ_CAPACITY_LENGTH)
	{
		printf("Actual write size(%d) is not equal to expect write size(%d)\n", actual_size, READ_CAPACITY_LENGTH);
	}
	/* 状态阶段 */
	if (getCswReply(usbdev->handle, usbdev->endpoint_in, expected_tag) == -2)
	{
		getSense(usbdev->handle, usbdev->endpoint_in, usbdev->endpoint_out);
	}
	max_block_addr = uchar2uint(&buffer[0]);		 /* 最大的块地址，从0开始数 */
	block_size = uchar2uint(&buffer[4]);			 /* 一个块的字节数,通常为512 */
	device_size = (max_block_addr + 1) * block_size; /* usb设备的总容量，单位：字节 */
	return device_size;
}

/****************************************************************************
Function: isUsbReady（）
Description: 询问设备是否准备好。
param1 : usbdev_node* usbdev        usb节点句柄
Return: 返回0说明准备好了，返回负数说明没准备好。
******************************************************************************/
int isUsbReady(usbdev_node *usbdev)
{

	int r;
	uint8_t cdb[16]; /* SCSI 命令 */
	uint8_t lun = 0;
	uint32_t expected_tag;
	memset(cdb, 0, sizeof(cdb));

	cdb[0] = READY_CMD; /* USB ready*/

	/* 批量传输的三个阶段，命令阶段，数据阶段，状态阶段 */
	/* 命令阶段 */
	r = sendCWBCommand(usbdev->handle, usbdev->endpoint_out, lun, cdb, LIBUSB_ENDPOINT_OUT, 0, &expected_tag);
	if (r < 0)
	{
		printf("Send CWB command failed\n");
		return -1;
	}
	/* 数据阶段 */

	/* 没有数据阶段了*/

	/* 状态阶段 */
	r = getCswReply(usbdev->handle, usbdev->endpoint_in, expected_tag);

	if (r == -2)
	{
		getSense(usbdev->handle, usbdev->endpoint_in, usbdev->endpoint_out);
	}
	return 0;
}

/************************************************************
Function: delUsbDev
Description: 删除某个usb节点
param1 : head                         usb设备链表头节点
param2 : usbdev                       要删除的usb设备节点
Return: 返回负数说明函数执行失败，返回0为成功
*************************************************************/
int delUsbDev(usbdev_node *head, usbdev_node *usbdev)
{
	int r;
	usbdev_node *pre = head;

	/* 寻找要删除的节点 */
	while (pre->next && pre->next->handle != usbdev->handle)
	{
		pre = pre->next;
	}
	if (pre->next == NULL)
	{
		printf("usb device can't delete\n");
		return -1;
	}

	/* 找到节点就删除节点，需要释放接口，释放设备handle内存,释放节点内存*/
	else
	{
		pre->next = pre->next->next;
		/* 释放接口 */
		r = libusb_release_interface(usbdev->handle, 0);
		if (r < 0)
		{
			printf("release interface failed:%s\n", libusb_strerror((enum libusb_error)r));
		}
		printf("\nclose device with ID %s\n", usbdev->stringID);

		/* 释放设备handle内存 */
		libusb_close(usbdev->handle);

		/* 释放节点内存 */
		free(usbdev);
	}
	return 0;
}
