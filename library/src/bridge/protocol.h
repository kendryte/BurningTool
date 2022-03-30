#include <stdint.h>
#include <stddef.h>
#include "global.h"

size_t create_pair_protocol(uint32_t bind_id, char *output, size_t size);
void pair_serial_ports_thread(KBCTX scope, const bool *const quit);
uint32_t handle_page(const char *buff, const size_t buff_size);

#define MAX_BUFF_SIZE 512
typedef struct binding_state
{
	char buffer[MAX_BUFF_SIZE];
	size_t buff_i;
	char packet[6];
	size_t packet_last;
} binding_state;
