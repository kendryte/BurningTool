#include "global.h"
#include "device-link-list.h"
#include <string.h>
#include <stdlib.h>

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

static inline port_link_element *find(KBCTX scope, const char *path)
{
	port_link_element *curs = NULL;

	lock(&scope->openDeviceList->exclusion);
	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next)
	{
		if (strcmp(curs->node->serial->path, path) == 0)
		{
			break;
		}
	}
	unlock(&scope->openDeviceList->exclusion);
	return curs;
}

kburnDeviceNode *get_device_by_serial_port_path(KBCTX scope, const char *path)
{
	port_link_element *ret = find(scope, path);
	return ret ? ret->node : NULL;
}

uint32_t kburnGetOpenPortCount()
{
	uint32_t i = 0;
	disposable_foreach_start(&lib_global_scope, item);
	const KBCTX scope = (KBCTX)item->userData;
	i += scope->openDeviceList->size;
	disposable_foreach_end(&lib_global_scope);
	return i;
}
