#include "bind-wait-list.h"
#include "context.h"
#include "components/device-link-list.h"
#include "global.h"

waiting_list_t *waiting_list_init()
{
	waiting_list_t *ret = calloc(1, sizeof(waiting_list_t));
	if (ret == NULL)
		return NULL;

	ret->mutex = lock_init();
	if (ret->mutex == NULL)
	{
		free(ret);
		return NULL;
	}

	return ret;
}

DECALRE_DISPOSE(waiting_list_deinit, waiting_list_t)
{
	lock_deinit(&context->mutex);
	free(context);
}
DECALRE_DISPOSE_END()

bool _should_insert_waitting_list(kburnDeviceNode *node)
{
	return node->serial->init && node->serial->isOpen && node->serial->isSwitchIsp && !node->serial->isUsbBound;
}

void _recreate_waitting_list(KBCTX scope)
{
	debug_trace_function();
	port_link_element *curs = NULL;

	memset(scope->waittingDevice->list, 0, sizeof(scope->waittingDevice->list));

	size_t itr = 0;
	for (curs = scope->openDeviceList->head; curs != NULL; curs = curs->next)
	{
		if (_should_insert_waitting_list(curs->node))
		{
			debug_print(KBURN_LOG_DEBUG, "  * %s", curs->node->serial->path);
			scope->waittingDevice->list[itr] = curs->node;

			itr++;
			if (itr >= MAX_WAITTING_DEVICES)
			{
				break;
			}
		}
	}
	debug_print(KBURN_LOG_DEBUG, "- %zu items", itr);
}
