#include "global.h"
#include "device-link-list.h"
#include <string.h>
#include <stdlib.h>

static inline bool _should_insert_waitting_list(kburnDeviceNode *node)
{
	return node->serial->init && node->serial->isOpen && node->serial->isSwitchIsp && !node->serial->isUsbBound;
}

static void _recreate_waitting_list(KBCTX scope)
{
	debug_print("\tportlist::recreate_waitting_list()");
	port_link_element *curs = NULL;

	lock(&scope->waittingDevice.lock);

	memset(scope->waittingDevice.list, 0, sizeof(scope->waittingDevice.list));

	size_t itr = 0;
	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next)
	{
		if (_should_insert_waitting_list(curs->node))
		{
			debug_print("\t\t%s", curs->node->serial->path);
			scope->waittingDevice.list[itr] = curs->node->serial;

			itr++;
			if (itr >= MAX_WAITTING_DEVICES)
			{
				break;
			}
		}
	}
	debug_print("\t%lu items", itr);

	unlock(&scope->waittingDevice.lock);
}

void recreate_waitting_list(KBCTX scope)
{
	lock(&scope->openDeviceList->exclusion);
	_recreate_waitting_list(scope);
	unlock(&scope->openDeviceList->exclusion);
}

void add_to_device_list(kburnDeviceNode *target)
{
	KBCTX scope = target->_scope;
	debug_print("add_to_device_list(0x%p) [size=%d]", (void *)target, scope->openDeviceList->size);
	port_link_element *ele = malloc(sizeof(port_link_element));
	ele->node = target;
	ele->prev = NULL;
	ele->next = NULL;

	lock(&scope->openDeviceList->exclusion);
	if (scope->openDeviceList->head)
	{
		ele->prev = scope->openDeviceList->tail;
		scope->openDeviceList->tail->next = ele;

		scope->openDeviceList->tail = ele;

		scope->openDeviceList->size++;
	}
	else
	{
		scope->openDeviceList->tail = scope->openDeviceList->head = ele;
		scope->openDeviceList->head->prev = NULL;
		scope->openDeviceList->head->next = NULL;

		scope->openDeviceList->size++;
	}

	if (_should_insert_waitting_list(target))
		_recreate_waitting_list(target->_scope);

	unlock(&scope->openDeviceList->exclusion);
}

static void do_delete(KBCTX scope, port_link_element *target)
{
	debug_print("\tportlist::do_delete(0x%p) [size=%d]", (void *)target, scope->openDeviceList->size);

	if (target->prev)
		target->prev->next = target->next;

	if (target->next)
		target->next->prev = target->prev;

	if (target == scope->openDeviceList->head)
		scope->openDeviceList->head = scope->openDeviceList->head->next;

	if (target == scope->openDeviceList->tail)
		scope->openDeviceList->tail = scope->openDeviceList->tail->prev;

	assert((scope->openDeviceList->size > 0) && "delete port from empty list");
	scope->openDeviceList->size--;

	if (_should_insert_waitting_list(target->node))
		_recreate_waitting_list(scope);

	free(target);
}

bool delete_from_device_list(kburnDeviceNode *target)
{
	KBCTX scope = target->_scope;
	debug_print("delete_from_device_list(0x%p) [size=%d]", (void *)target, scope->openDeviceList->size);
	lock(&scope->openDeviceList->exclusion);
	for (port_link_element *curs = scope->openDeviceList->head; curs != NULL; curs = curs->next)
	{
		if (curs->node == target)
		{
			do_delete(scope, curs);
			unlock(&scope->openDeviceList->exclusion);
			return true;
		}
	}
	unlock(&scope->openDeviceList->exclusion);
	debug_print("  - not found");
	return false;
}

static inline port_link_element *find_serial_device(KBCTX scope, const char *path)
{
	port_link_element *curs = NULL;

	lock(&scope->openDeviceList->exclusion);
	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next)
	{
		if (!curs->node->serial->init)
			continue;
		if (strcmp(curs->node->serial->path, path) == 0)
			break;
	}
	unlock(&scope->openDeviceList->exclusion);
	return curs;
}

static inline port_link_element *find_usb_device_by_vidpidpath(KBCTX scope, uint16_t vid, uint8_t pid, const uint8_t *path)
{
	port_link_element *curs = NULL;

	lock(&scope->openDeviceList->exclusion);
	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next)
	{
		if (curs->node->usb->deviceInfo.idVendor == vid && curs->node->usb->deviceInfo.idProduct == pid && strncmp((char *)path, (char *)curs->node->usb->deviceInfo.path, MAX_PATH_LENGTH) == 0)
			break;
	}
	unlock(&scope->openDeviceList->exclusion);
	return curs;
}

kburnDeviceNode *get_device_by_serial_port_path(KBCTX scope, const char *path)
{
	port_link_element *ret = find_serial_device(scope, path);
	return ret ? ret->node : NULL;
}

kburnDeviceNode *get_device_by_usb_port_path(KBCTX scope, uint16_t vid, uint8_t pid, const uint8_t *path)
{
	port_link_element *ret = find_usb_device_by_vidpidpath(scope, vid, pid, path);
	return ret ? ret->node : NULL;
}

static bool has_bind_id_used(KBCTX scope, uint32_t id)
{
	port_link_element *curs = NULL;

	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next)
	{
		if (curs->node->bind_id == id)
			return true;
	}
	return false;
}

void alloc_new_bind_id(kburnDeviceNode *node)
{
	uint32_t id = 0;

	lock(&node->_scope->openDeviceList->exclusion);

	do
	{
		id = rand();
	} while (has_bind_id_used(node->_scope, id));
	node->bind_id = id;

	unlock(&node->_scope->openDeviceList->exclusion);
}
