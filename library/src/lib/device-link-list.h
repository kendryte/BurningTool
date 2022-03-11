#include "canaan-burn.h"

typedef struct port_link_element
{
	struct port_link_element *prev;
	kburnDeviceNode *node;
	struct port_link_element *next;
} port_link_element;

struct port_link_list
{
	port_link_element *head;
	port_link_element *tail;
	uint32_t size;
	volatile int exclusion;
};
