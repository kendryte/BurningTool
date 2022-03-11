#include "usb.h"

/****************************************************
Function: getDevEndpoint
Description: 获取usb设备的端点。in和out
Called By:   由函数serchOpenDevice()调用
param1 : dev         枚举出来并符合vid pid的usb设备
param2 : current     当前设备节点
Return: 返回负数说明获取端点失败，返回0为成功
*******************************************************/
int get_device_info(libusb_device *dev, usbdev_node *current)
{
	int r;
	int k, j;
	struct libusb_config_descriptor *conf_desc;
	const struct libusb_endpoint_descriptor *endpoint;

	/* 获取配置描述符，里面包含端点信息 */
	r = libusb_get_config_descriptor(dev, 0, &conf_desc);
	if (r < 0)
	{
		debug_print("Get config descriptor failed: %s", libusb_strerror((enum libusb_error)r));
		return -1;
	}

	/* 默认使用第一个接口 usb_interface[0]*/
	for (j = 0; j < conf_desc->usb_interface[0].num_altsetting; j++)
	{
		for (k = 0; k < conf_desc->usb_interface[0].altsetting[j].bNumEndpoints; k++)
		{
			endpoint = &conf_desc->usb_interface[0].altsetting[j].endpoint[k];
			debug_print("Endpoint[%d].address: %02X", k, endpoint->bEndpointAddress);
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
	libusb_free_config_descriptor(conf_desc);
	return 0;
}

void destroy_usb_port(KBCTX UNUSED(scope), kburnDeviceNode *device)
{
	kburnUsbDeviceNode *usb = device->usb;
	if (!usb->init)
		return;
	debug_print("destroy_usb_port(usn[0x%p, %s])", (void *)usb, "");
}
