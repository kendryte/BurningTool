#include "usb.h"

typedef struct
{
	uint8_t endpoint_in;
	uint8_t endpoint_out;
	int error;
} endpoint_t;

/****************************************************
Function: get_endpoint
Description: 获取usb设备的端点。in和out
param1 : dev         枚举出来并符合vid pid的usb设备
param2 : current     当前设备节点
Return: 返回负数说明获取端点失败，返回0为成功
*******************************************************/
static endpoint_t get_endpoint(libusb_device *dev)
{
	debug_print("get_endpoint(%p)", (void *)dev);
	int r;
	int k, j;
	struct libusb_config_descriptor *conf_desc;
	const struct libusb_endpoint_descriptor *endpoint;
	endpoint_t ret = {0, 0, 0};

	/* 获取配置描述符，里面包含端点信息 */
	r = libusb_get_config_descriptor(dev, 0, &conf_desc);
	if (r < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("get config descriptor failed", r);
		ret.error = r;
		return ret;
	}

	/* 默认使用第一个接口 usb_interface[0]*/
	for (j = 0; j < conf_desc->usb_interface[0].num_altsetting; j++)
	{
		for (k = 0; k < conf_desc->usb_interface[0].altsetting[j].bNumEndpoints; k++)
		{
			endpoint = &conf_desc->usb_interface[0].altsetting[j].endpoint[k];
			debug_print("\tendpoint[%d].address: %02X", k, endpoint->bEndpointAddress);

			/* 使用批量传输的端点 */
			if (endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK & LIBUSB_TRANSFER_TYPE_BULK)
			{
				if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN)
				{
					ret.endpoint_in = endpoint->bEndpointAddress;
					debug_print(" * endpoint_in = %02X", ret.endpoint_in);
					if (ret.endpoint_out)
						goto _quit;
				}
				else
				{
					ret.endpoint_out = endpoint->bEndpointAddress;
					debug_print(" * endpoint_out = %02X", ret.endpoint_out);
					if (ret.endpoint_in)
						goto _quit;
				}
			}
		}
	}

_quit:
	libusb_free_config_descriptor(conf_desc);
	return ret;
}

void destroy_usb_port(KBCTX scope, kburnDeviceNode *device)
{
	close_single_usb_port(scope, device);
}

kburn_err_t create_usb_port(KBCTX UNUSED(scope), kburnDeviceNode *device)
{
	kburnUsbDeviceNode *usb = device->usb;
	if (device->usb->init)
		return KBurnNoErr;

	usb->isOpen = false;
	usb->isClaim = false;
	usb->init = true;

	return KBurnNoErr;
}

kburn_err_t open_single_usb_port(KBCTX scope, struct libusb_device *dev)
{
	debug_print("open_single_usb_port(%p)", (void *)dev);
	assert(scope->usb->libusb && "usb subsystem is not inited");

	int rusb = 0;
	kburn_err_t risp = 0;

	kburnDeviceNode *node;
	kburn_err_t r = create_empty_device_instance(scope, &node);
	if (r != KBurnNoErr)
		return r;

	node->usb->device = dev;

	rusb = usb_get_vid_pid_path(dev, &node->usb->deviceInfo.idVendor, &node->usb->deviceInfo.idProduct, node->usb->deviceInfo.path);
	if (rusb < LIBUSB_SUCCESS)
		goto cleanup_single_port;

	rusb = libusb_open(dev, &node->usb->handle); // TODO: leak
	if (rusb < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("open_single_usb_port: libusb_open", rusb);
		goto cleanup_single_port;
	}
	node->usb->isOpen = true;
	debug_print("usb port open success");

	rusb = usb_get_device_serial(dev, node->usb->handle, node->usb->deviceInfo.strSerial);
	if (rusb < LIBUSB_SUCCESS)
	{
		goto cleanup_single_port;
	}

	endpoint_t ret2 = get_endpoint(dev);
	if (ret2.error < LIBUSB_SUCCESS)
	{
		rusb = ret2.error;
		goto cleanup_single_port;
	}
	node->usb->deviceInfo.endpoint_in = ret2.endpoint_in;
	node->usb->deviceInfo.endpoint_out = ret2.endpoint_out;

	debug_print("libusb_open: handle=%p", (void *)node->usb->handle);

	rusb = libusb_set_auto_detach_kernel_driver(node->usb->handle, 1);
	if (rusb < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("open_single_usb_port: system not support auto detach kernel driver", rusb);
		// no return here
	}

	rusb = libusb_claim_interface(node->usb->handle, 0);
	if (rusb < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("open_single_usb_port: libusb_claim_interface", rusb);
		goto cleanup_single_port;
	}
	node->usb->isClaim = true;

	debug_print("libusb_claim_interface success");

	risp = usb_device_hello(node);
	if (risp != KBurnNoErr)
		goto cleanup_single_port;

	debug_print("hello success.");

	alloc_new_bind_id(node);
	debug_print("	bind id = %d", node->bind_id);

	for (int try = 20; try > 0; try--)
	{
		risp = usb_device_serial_bind(node);
		if (risp != KBurnNoErr)
			goto cleanup_single_port;
	}

	goto success_single_port;
cleanup_single_port:
	if (node != NULL)
		device_instance_collect(scope, node);

	if (rusb != LIBUSB_SUCCESS)
		return rusb | KBURN_ERROR_KIND_USB;
	else if (risp)
		return risp;
	else
		return KBurnWiredError | KBURN_ERROR_KIND_COMMON;

success_single_port:
	node->usb->init = true;
	return KBurnNoErr;
}

kburn_err_t close_single_usb_port(KBCTX scope, kburnDeviceNode *dev)
{
	if (!scope->usb->libusb)
		return KBurnNoErr;

	debug_print("destroy_usb_port(usb[0x%p: %s])", (void *)dev->usb->device, dev->usb->deviceInfo.strSerial);

	if (!dev->usb->init)
		return KBurnNoErr;

	if (dev->usb->isClaim)
	{
		dev->usb->isClaim = false;
		libusb_release_interface(dev->usb->handle, 0);
	}

	if (dev->usb->isOpen)
	{
		dev->usb->isOpen = false;
		libusb_close(dev->usb->handle);
		dev->usb->handle = NULL;
		dev->usb->device = NULL;
	}

	dev->usb->init = false;
	return KBurnNoErr;
}

kburnDeviceNode *usb_device_find(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *path)
{
	return get_device_by_usb_port_path(scope, vid, pid, path);
}
