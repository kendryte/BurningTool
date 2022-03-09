#include "serial.h"

on_device_handle disconnect_callback = NULL;
void kburnRegisterDisconnectCallback(on_device_handle callback)
{
	disconnect_callback = callback;
}

void _destroy_port_void(void *node)
{
	destroy_port((kburnSerialNode *)node);
}

void destroy_port(kburnSerialNode *node)
{
	debug_print("destroy_port(%p[%s])", (void *)node, node->path);
	if (disconnect_callback)
	{
		debug_print("\tdisconnect_callback()");
		disconnect_callback(node);
	}

	global_resource_unregister(_destroy_port_void, node);

	if (node->isOpen)
	{
		debug_print("\tser_close()");
		ser_close(node->m_dev_handle);
	}

	pop_from_port_list(node);

	if (node->path != NULL)
	{
		debug_print("\tfree(node->path = %s)", node->path);
		free((void *)node->path);
	}

	if (node->errorMessage != NULL)
	{
		debug_print("\tfree(node->errorMessage = %s)", node->errorMessage);
		free(node->errorMessage);
	}

	if (node->m_dev_handle != NULL)
	{
		debug_print("\tser_destroy()");
		ser_destroy(node->m_dev_handle);
	}

	debug_print("\tfree(node)");
	free(node);
}

kburnSerialNode *create_port(const char *path)
{
	debug_print("create_port(%s)", path);
	kburnSerialNode *node = malloc(sizeof(kburnSerialNode));
	memset(node, 0, sizeof(kburnSerialNode));
	node->path = strdup(path);
	add_to_port_list(node);

	global_resource_register(_destroy_port_void, node);
	return node;
}

kburnSerialNode *kburnOpen(const char *path)
{
	debug_print("kburnOpen(%s)", path);
	on_device_attach(path);

	return find_from_port_list(path);
}
