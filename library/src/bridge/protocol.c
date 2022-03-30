#include "protocol.h"
#include <stdio.h>
#include <string.h>

#define SIGNATURE "{bind-usb-uart: 0x"

size_t create_pair_protocol(uint32_t bind_id, char *output, size_t size)
{
	snprintf(output, size, SIGNATURE "%.8x}", bind_id);
	uint32_t sum = 0;
	for (size_t i = 0; i < strlen(output); i++)
	{
		sum += output[i];
	}
	return snprintf(output, size, SIGNATURE "%.8x = %d}", bind_id, sum);
}

uint32_t handle_page(const char *buff, const size_t buff_size)
{
	debug_print("handle page: %.*s", (int)buff_size, buff);
	if (!strncmp(buff, SIGNATURE, buff_size))
		return false;

	uint32_t bind_id = strtoul(buff + strlen(SIGNATURE), NULL, 16);
	debug_print("  -> bind_id = %u", bind_id);

	char vbuff[buff_size * 2];
	create_pair_protocol(bind_id, vbuff, buff_size * 2);
	if (strncmp(buff, vbuff, buff_size) != 0)
	{
		debug_print("!! verify failed: %s", vbuff);
		return 0;
	}

	return bind_id;
}
