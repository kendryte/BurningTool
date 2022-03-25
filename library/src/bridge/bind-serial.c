#include "protocol.h"
#include "global.h"
#include "serial.h"

static void push_buffer(char *tgt, size_t *tgt_i, const char *src, const size_t src_size)
{
	memcpy(tgt + *tgt_i, src, src_size);
	*tgt_i += src_size;
}
static void cut_buffer_start(char *buf, size_t *tgt_i, const size_t pos)
{
	memmove(buf, buf + pos, *tgt_i - pos);
	*tgt_i -= pos;
}
static bool verify4chr(char c1, char c2, char c3, char c4, char sum)
{
	int vsum = c1 + c2 + c3 + c4;
	return vsum == (int)sum;
}

static bool handle_one_device(kburnSerialDeviceNode *dev)
{
#define MAX_INPUT 64

	size_t in_buffer_size = 0;
	binding_state *b = dev->binding;

	ser_available(dev->m_dev_handle, &in_buffer_size);

	if (in_buffer_size == 0)
		return false;

	// size_t remaining = MAX_BUFF_SIZE - b->buff_i;
	// if (remaining < in_buffer_size)
	// {
	// 	debug_print("serial binding buffer overflow");
	// 	b->buff_i = 0;
	// 	if (in_buffer_size > MAX_BUFF_SIZE)
	// 		in_buffer_size = MAX_BUFF_SIZE;
	// }

	size_t tbuff_start = b->packet_last;
	b->packet_last = 0;
	size_t tbuff_size = tbuff_start + in_buffer_size;
	char tbuff[tbuff_size];
	memcpy(tbuff, b->packet, tbuff_start);

	int err = ser_read(dev->m_dev_handle, tbuff + tbuff_start, in_buffer_size, NULL);
	if (err != 0)
	{
		copy_last_serial_io_error(dev->parent, err);
		return false;
	}

	debug_print("\t%ld is new", in_buffer_size);

	for (size_t i = 0; i < tbuff_size; i++)
	{
		if (tbuff[i] != '\xff')
			continue;

		if (i + 6 > tbuff_size)
		{
			push_buffer(b->packet, &b->packet_last, tbuff + i, tbuff_size - i);
			break;
		}

		if (verify4chr(b->buffer[i + 1], b->buffer[i + 2], b->buffer[i + 3], b->buffer[i + 4], b->buffer[i + 5]))
		{
			push_buffer(b->buffer, &b->buff_i, tbuff + i + 1, 4);
			i += 5;
		}
	}

	while (true)
	{
		int found_start = -1, found_end = -1;
		size_t work_size = (b->buff_i / 6) * 6;
		for (size_t i = 0; i < work_size; i++)
		{
			char ch = b->buffer[i];
			if (ch == '{')
			{
				found_start = i;
			}
			else if (found_start >= 0 && ch == '}')
			{
				found_end = i;
				break;
			}
		}

		if (found_start < 0)
		{
			b->buff_i = 0;
			return false;
		}

		if (found_end < 0)
		{
			if (found_start > 0)
			{
				cut_buffer_start(b->buffer, &b->buff_i, found_start);
			}
			return false;
		}

		bool hit = handle_page(dev, b->buffer + found_start, found_end - found_start);

		if (hit)
			return true;

		if ((uint32_t)found_end + 1 > b->buff_i)
		{
			b->buff_i = 0;
		}
		else
		{
			cut_buffer_start(b->buffer, &b->buff_i, found_end + 1);
		}
	}
	return false;
}

void pair_serial_ports_thread(KBCTX scope, const bool *const quit)
{
	bool need_recreate = false;
	int delay = 100;
	while (!*quit)
	{
		int current_size = 0;

		lock(&scope->waittingDevice.lock);

		for (kburnSerialDeviceNode **ptr = scope->waittingDevice.list; *ptr != NULL; ptr++)
		{
			kburnSerialDeviceNode *dev = *ptr;

			if (dev->binding == NULL)
			{
				dev->binding = calloc(1, sizeof(binding_state));
			}

			if (handle_one_device(*ptr))
			{
				need_recreate = true;
			}
			current_size++;
		}

		unlock(&scope->waittingDevice.lock);

		if (need_recreate)
		{
			recreate_waitting_list(scope);
			continue;
		}

		if (current_size > 0)
			delay = 100;
		else if (delay < 5000)
			delay += 10;

		do_sleep(delay);
	}
}
