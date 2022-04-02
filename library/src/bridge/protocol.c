#include "protocol.h"
#include "basic/string.h"
#include <stdio.h>
#include <string.h>

#define SIGNATURE "{bind-usb-uart: 0x"

size_t create_pair_protocol(uint32_t bind_id, char *output, size_t size)
{
	m_snprintf(output, size, SIGNATURE "%.8x}", bind_id);
	uint32_t sum = 0;
	for (size_t i = 0; i < strlen(output); i++)
	{
		sum += output[i];
	}
	return m_snprintf(output, size, SIGNATURE "%.8x = %d}", bind_id, sum);
}

uint32_t handle_page(const char *buff, const size_t buff_size)
{
	debug_trace_function("%.*s", (int)buff_size, buff);
	if (!strncmp(buff, SIGNATURE, buff_size))
		return false;

	uint32_t bind_id = strtoul(buff + strlen(SIGNATURE), NULL, 16);
	debug_print(KBURN_LOG_DEBUG, "  -> bind_id = %u (%X)", bind_id, bind_id);

	char vbuff[buff_size * 2];
	create_pair_protocol(bind_id, vbuff, buff_size * 2);
	if (strncmp(buff, vbuff, buff_size) != 0)
	{
		debug_print(KBURN_LOG_DEBUG, "!! verify failed: %s", vbuff);
		return 0;
	}

	return bind_id;
}
