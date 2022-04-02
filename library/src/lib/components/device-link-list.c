#include "device-link-list.h"
#include "bind-wait-list.h"
#include "global.h"
#include <stdlib.h>
#include <string.h>

port_link_list *port_link_list_init() {
	DeferEnabled;

	port_link_list *ret = calloc(1, sizeof(port_link_list));
	DeferFree(ret);
	if (ret == NULL)
		return NULL;

	ret->exclusion = lock_init();
	if (ret == NULL)
		return NULL;

	DeferAbort;
	return ret;
}
DECALRE_DISPOSE(port_link_list_deinit, port_link_list) {
	lock_deinit(&context->exclusion);
	free(context);
}
DECALRE_DISPOSE_END()

void recreate_waitting_list(KBCTX scope) {
	autolock(scope->openDeviceList->exclusion);
	_recreate_waitting_list(scope);
}

void add_to_device_list(kburnDeviceNode *target) {
	KBCTX scope = target->_scope;
	debug_trace_function("0x%p, [size=%d]", (void *)target, scope->openDeviceList->size);
	port_link_element *ele = malloc(sizeof(port_link_element));
	ele->node = target;
	ele->prev = NULL;
	ele->next = NULL;

	autolock(scope->openDeviceList->exclusion);
	if (scope->openDeviceList->head) {
		ele->prev = scope->openDeviceList->tail;
		scope->openDeviceList->tail->next = ele;

		scope->openDeviceList->tail = ele;

		scope->openDeviceList->size++;
	} else {
		scope->openDeviceList->tail = scope->openDeviceList->head = ele;
		scope->openDeviceList->head->prev = NULL;
		scope->openDeviceList->head->next = NULL;

		scope->openDeviceList->size++;
	}

	dispose_list_add(target->disposable_list, toDisposable(delete_from_device_list, target));

	if (_should_insert_waitting_list(target))
		_recreate_waitting_list(target->_scope);
}

static void do_delete(KBCTX scope, port_link_element *target) {
	debug_trace_function("\t0x%p [size=%d]", (void *)target, scope->openDeviceList->size);

	if (target->prev)
		target->prev->next = target->next;

	if (target->next)
		target->next->prev = target->prev;

	if (target == scope->openDeviceList->head)
		scope->openDeviceList->head = scope->openDeviceList->head->next;

	if (target == scope->openDeviceList->tail)
		scope->openDeviceList->tail = scope->openDeviceList->tail->prev;

	m_assert(scope->openDeviceList->size > 0, "delete port from empty list");
	scope->openDeviceList->size--;

	if (_should_insert_waitting_list(target->node)) {
		_recreate_waitting_list(scope);
	}

	free(target);
}

DECALRE_DISPOSE(delete_from_device_list, kburnDeviceNode) {
	KBCTX scope = context->_scope;
	debug_trace_function("0x%p [size=%d]", (void *)context, scope->openDeviceList->size);
	autolock(scope->openDeviceList->exclusion);
	for (port_link_element *curs = scope->openDeviceList->head; curs != NULL; curs = curs->next) {
		if (curs->node == context) {
			do_delete(scope, curs);
			return;
		}
	}
	debug_print(KBURN_LOG_WARN, "  - not found");
	return;
}
DECALRE_DISPOSE_END()

static inline port_link_element *find_serial_device(KBCTX scope, const char *path) {
	port_link_element *curs = NULL;

	autolock(scope->openDeviceList->exclusion);
	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next) {
		if (!curs->node->serial->init)
			continue;
		if (strcmp(curs->node->serial->path, path) == 0)
			break;
	}
	return curs;
}

static inline port_link_element *find_usb_device_by_vidpidpath(KBCTX scope, uint16_t vid, uint8_t pid, const uint8_t *path) {
	port_link_element *curs = NULL;

	autolock(scope->openDeviceList->exclusion);
	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next) {
		if (curs->node->usb->deviceInfo.idVendor == vid && curs->node->usb->deviceInfo.idProduct == pid &&
			strncmp((char *)path, (char *)curs->node->usb->deviceInfo.path, MAX_PATH_LENGTH) == 0)
			break;
	}
	return curs;
}

kburnDeviceNode *get_device_by_serial_port_path(KBCTX scope, const char *path) {
	port_link_element *ret = find_serial_device(scope, path);
	return ret ? ret->node : NULL;
}

kburnDeviceNode *get_device_by_usb_port_path(KBCTX scope, uint16_t vid, uint8_t pid, const uint8_t *path) {
	port_link_element *ret = find_usb_device_by_vidpidpath(scope, vid, pid, path);
	return ret ? ret->node : NULL;
}

static kburnDeviceNode *has_bind_id_used(KBCTX scope, uint32_t id) {
	port_link_element *curs = NULL;

	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next) {
		if (curs->node->bind_id == id)
			return curs->node;
	}
	return NULL;
}

void alloc_new_bind_id(kburnDeviceNode *node) {
	uint32_t id = 0;

	autolock(node->_scope->openDeviceList->exclusion);

	do {
		do {
			id = rand();
		} while (id == 0);
	} while (has_bind_id_used(node->_scope, id));
	node->bind_id = id;
}

kburnDeviceNode *get_device_by_bind_id(KBCTX scope, uint32_t id) { return has_bind_id_used(scope, id); }
