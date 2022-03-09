#include <global.h>
#include <string.h>
#include <stdlib.h>

typedef struct port_link_list
{
	struct port_link_list *prev;
	kburnSerialNode *node;
	struct port_link_list *next;
} port_link_list;

static port_link_list *head = NULL;
static port_link_list *tail = NULL;
static uint32_t size = 0;

static volatile int exclusion = 0;

void add_to_port_list(kburnSerialNode *target)
{
	debug_print("add_to_port_list(0x%p) [size=%d]", (void *)target, size);
	port_link_list *ele = malloc(sizeof(port_link_list));
	ele->node = target;
	ele->prev = NULL;
	ele->next = NULL;

	lock(&exclusion);
	if (head)
	{
		ele->prev = tail;
		tail->next = ele;

		tail = ele;

		size++;
	}
	else
	{
		tail = head = ele;
		head->prev = NULL;
		head->next = NULL;

		size++;
	}
	unlock(&exclusion);
}

static void do_delete(port_link_list *target)
{
	debug_print("\tportlist::do_delete(0x%p) [size=%d]", (void *)target, size);

	if (target->prev)
		target->prev->next = target->next;

	if (target->next)
		target->next->prev = target->prev;

	if (target == head)
		head = head->next;

	if (target == tail)
		tail = tail->prev;

	assert((size > 0) && "delete port from empty list");
	size--;

	free(target);
}

bool pop_from_port_list(kburnSerialNode *target)
{
	debug_print("pop_from_port_list(0x%p) [size=%d]", (void *)target, size);
	lock(&exclusion);
	for (port_link_list *curs = head; curs != NULL; curs = curs->next)
	{
		if (curs->node == target)
		{
			do_delete(curs);
			unlock(&exclusion);
			return true;
		}
	}
	unlock(&exclusion);
	debug_print("  - not found");
	return false;
}

static inline port_link_list *find(const char *path)
{
	port_link_list *curs = NULL;

	lock(&exclusion);
	for (curs = head; curs != NULL; curs = curs->next)
	{
		if (strcmp(curs->node->path, path) == 0)
		{
			break;
		}
	}
	unlock(&exclusion);
	return curs;
}

kburnSerialNode *find_from_port_list(const char *path)
{
	port_link_list *ret = find(path);
	return ret ? ret->node : NULL;
}

uint32_t kburnGetOpenPortCount()
{
	return size;
}
