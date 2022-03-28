#include "usb.h"

/****************************************************
Function: get_endpoint
Description: 获取usb设备的端点。in和out
param1 : dev         枚举出来并符合vid pid的usb设备
param2 : current     当前设备节点
Return: 返回负数说明获取端点失败，返回0为成功
*******************************************************/
static int get_endpoint(libusb_device *dev, uint8_t *out_endpoint_in, uint8_t *out_endpoint_out)
{
	debug_print("get_endpoint(%p)", (void *)dev);
	int k, j;
	struct libusb_config_descriptor *conf_desc;
	const struct libusb_endpoint_descriptor *endpoint;
	*out_endpoint_in = 0;
	*out_endpoint_out = 0;

	/* 获取配置描述符，里面包含端点信息 */
	IfUsbErrorLogReturn(
		libusb_get_config_descriptor(dev, 0, &conf_desc));

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
					*out_endpoint_in = endpoint->bEndpointAddress;
					debug_print(" * endpoint_in = 0x%02X", *out_endpoint_in);
					if (*out_endpoint_out)
						goto _quit;
				}
				else
				{
					*out_endpoint_out = endpoint->bEndpointAddress;
					debug_print(" * endpoint_out = 0x%02X", *out_endpoint_out);
					if (*out_endpoint_in)
						goto _quit;
				}
			}
		}
	}

_quit:
	libusb_free_config_descriptor(conf_desc);
	return LIBUSB_SUCCESS;
}

DECALRE_DISPOSE(destroy_usb_port, kburnUsbDeviceNode)
{
	debug_print("destroy_usb_port(USB[0x%p: %s])", (void *)context->device, _debug_path_string(context->deviceInfo.path));

	lock(context->mutex);
	if (context->isClaim)
	{
		context->isClaim = false;
		libusb_release_interface(context->handle, 0);
	}

	if (context->isOpen)
	{
		context->isOpen = false;
		libusb_close(context->handle);
		context->handle = NULL;
		context->device = NULL;
	}

	context->init = false;
	unlock(context->mutex);
	lock_deinit(&context->mutex);
}
DECALRE_DISPOSE_END()

kburn_err_t open_single_usb_port(KBCTX scope, struct libusb_device *dev)
{
	DeferEnabled;

	debug_print(GREEN("open_single_usb_port") "(%p)", (void *)dev);
	m_assert(scope->usb->libusb, "usb subsystem is not inited");

	int r;
	kburnDeviceNode *node;
	IfErrorReturn(
		create_empty_device_instance(scope, &node));
	DeferDispose(scope->disposables, node, destroy_device);
	DeferDispose(node->disposable_list, node->usb, destroy_usb_port);

	node->usb->mutex = CheckNull(lock_init());
	node->usb->device = dev;

	IfUsbErrorReturn(
		usb_get_vid_pid_path(dev, &node->usb->deviceInfo.idVendor, &node->usb->deviceInfo.idProduct, node->usb->deviceInfo.path));

	IfUsbErrorLogReturn(
		libusb_open(dev, &node->usb->handle));
	node->usb->isOpen = true;
	debug_print("usb port open success, handle=%p", (void *)node->usb->handle);

	r = libusb_detach_kernel_driver(node->usb->handle, 0);
	if (r < LIBUSB_SUCCESS)
	{
		debug_print_libusb_error("open_single_usb_port: system not support detach kernel driver", r);
		// no return here
	}

	usb_get_device_serial(dev, node->usb->handle, node->usb->deviceInfo.strSerial);

	IfUsbErrorReturn(
		get_endpoint(dev, &node->usb->deviceInfo.endpoint_in, &node->usb->deviceInfo.endpoint_out));

	libusb_clear_halt(node->usb->handle, 0);
	IfUsbErrorLogReturn(
		libusb_claim_interface(node->usb->handle, 0));
	node->usb->isClaim = true;
	debug_print("libusb_claim_interface success");

	IfErrorReturn(
		usb_device_hello(node));
	debug_print("hello success.");

	add_to_device_list(node);

	alloc_new_bind_id(node);
	debug_print("	bind id = %d", node->bind_id);

	IfErrorReturn(
		usb_device_serial_bind(node));

	node->usb->init = true;

	DeferAbort;
	return KBurnNoErr;
}

kburnDeviceNode *usb_device_find(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *path)
{
	return get_device_by_usb_port_path(scope, vid, pid, path);
}
