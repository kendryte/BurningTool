#include "protocol.h"
#include <stdio.h>
#include <string.h>

size_t create_pair_protocol(uint32_t bind_id, char *output, size_t size)
{
	snprintf(output, size, "{bind-usb-uart: 0x%.8x}", bind_id);
	uint32_t sum = 0;
	for (size_t i = 0; i < strlen(output); i++)
	{
		sum += output[i];
	}
	return snprintf(output, size, "{bind-usb-uart: 0x%.8x = %d}", bind_id, sum);
}
