#include "serial.h"

void copy_last_error(kburnSerialNode *node)
{
	if (node->errorMessage != NULL)
	{
		debug_print("copy_last_error: free last");
		free(node->errorMessage);
	}
	node->errorMessage = strdup(sererr_last());
	debug_print("copy_last_error: %s", node->errorMessage);
}
