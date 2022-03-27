#include "canaan-burn/canaan-burn.h"
#include "../basic/lock.h"

typedef struct port_link_element
{
	struct port_link_element *prev;
	kburnDeviceNode *node;
	struct port_link_element *next;
} port_link_element;

typedef struct port_link_list
{
	port_link_element *head;
	port_link_element *tail;
	uint32_t size;
	kb_mutex_t exclusion;
} port_link_list;

port_link_list *port_link_list_init();
DECALRE_DISPOSE_HEADER(port_link_list_deinit, port_link_list);
