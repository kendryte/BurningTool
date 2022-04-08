#include "../serial/move-device.h"
#include "../serial/private-types.h"
#include "basic/sleep.h"
#include "bind-wait-list.h"
#include "device.h"
#include "protocol.h"
#include "components/device-link-list.h"
#include "debug/print.h"

static void push_buffer(char *tgt, size_t *tgt_i, const char *src, const size_t src_size) {
	memcpy(tgt + *tgt_i, src, src_size);
	*tgt_i += src_size;
}
static void cut_buffer_start(char *buf, size_t *tgt_i, const size_t pos) {
	memmove(buf, buf + pos, *tgt_i - pos);
	*tgt_i -= pos;
}
static bool verify4chr(char c1, char c2, char c3, char c4, char sum) {
	int vsum = c1 + c2 + c3 + c4;
	return vsum == (int)sum;
}

static uint32_t handle_one_device(kburnSerialDeviceNode *dev) {
#define MAX_INPUT 64

	size_t in_buffer_size = 0;
	binding_state *b = dev->binding;

	ser_available(dev->m_dev_handle, &in_buffer_size);

	if (in_buffer_size == 0) {
		return false;
	}

	size_t tbuff_start = b->packet_last;
	b->packet_last = 0;
	size_t tbuff_size = tbuff_start + in_buffer_size;
	char tbuff[tbuff_size];
	memcpy(tbuff, b->packet, tbuff_start);

	int err = ser_read(dev->m_dev_handle, tbuff + tbuff_start, in_buffer_size, NULL);
	if (err != 0) {
		copy_last_serial_io_error(dev->parent, err);
		return false;
	}

	debug_print(KBURN_LOG_DEBUG, "\t" FMT_SIZET " in serial buffer", in_buffer_size);

	for (size_t i = 0; i < tbuff_size; i++) {
		if (tbuff[i] != '\xff') {
			continue;
		}

		if (i + 6 > tbuff_size) {
			push_buffer(b->packet, &b->packet_last, tbuff + i, tbuff_size - i);
			break;
		}

		if (verify4chr(b->buffer[i + 1], b->buffer[i + 2], b->buffer[i + 3], b->buffer[i + 4], b->buffer[i + 5])) {
			push_buffer(b->buffer, &b->buff_i, tbuff + i + 1, 4);
			i += 5;
		}
	}

	while (true) {
		int found_start = -1, found_end = -1;
		size_t work_size = (b->buff_i / 6) * 6;
		for (size_t i = 0; i < work_size; i++) {
			char ch = b->buffer[i];
			if (ch == '{') {
				found_start = i;
			} else if (found_start >= 0 && ch == '}') {
				found_end = i;
				break;
			}
		}

		if (found_start < 0) {
			b->buff_i = 0;
			return false;
		}

		if (found_end < 0) {
			if (found_start > 0) {
				cut_buffer_start(b->buffer, &b->buff_i, found_start);
			}
			return false;
		}

		uint32_t bind_id = handle_page(b->buffer + found_start, found_end - found_start);

		if (bind_id > 0) {
			return bind_id;
		}

		if ((uint32_t)found_end + 1 > b->buff_i) {
			b->buff_i = 0;
		} else {
			cut_buffer_start(b->buffer, &b->buff_i, found_end + 1);
		}
	}
	return false;
}

void pair_serial_ports_thread(void *UNUSED(ctx), KBCTX scope, const bool *const quit) {
	int delay = 100;
	while (!*quit) {
		int item_waitting_pair = 0;

		lock(scope->waittingDevice->mutex);

		for (kburnDeviceNode **ptr = scope->waittingDevice->list; *ptr != NULL; ptr++) {
			kburnDeviceNode *serial_node = *ptr;
			kburnSerialDeviceNode *dev = serial_node->serial;
			if (!dev->init) { // TODO: need lock with deinit
				continue;
			}

			if (dev->binding == NULL) {
				dev->binding = calloc(1, sizeof(binding_state));
			}

			uint32_t found_bind = handle_one_device(dev);
			if (found_bind > 0) {
				dev->isUsbBound = true;
				free(dev->binding);
				dev->binding = NULL;

				recreate_waitting_list(scope);

				kburnDeviceNode *new_usb_node = get_device_by_bind_id(scope, found_bind);
				if (new_usb_node == NULL) {
					debug_print(KBURN_LOG_ERROR, COLOR_FMT("bind target usb port gone, bind_id=%d, maybe disconnected?"), COLOR_ARG(RED, found_bind));
					continue;
				}
				copy_serial_device(serial_node, new_usb_node);

				continue;
			}
			item_waitting_pair++;
		}

		unlock(scope->waittingDevice->mutex);

		if (item_waitting_pair > 0) {
			delay = 100;
		} else if (delay < 1000) {
			delay += 10;
		}

		do_sleep(delay);
	}
}
