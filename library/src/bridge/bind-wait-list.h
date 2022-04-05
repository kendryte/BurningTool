#pragma once

#include "basic/disposable.h"
#include "basic/lock.h"
#include "context.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_WAITTING_DEVICES 32
typedef struct waiting_list {
	kb_mutex_t mutex;
	kburnDeviceNode *list[MAX_WAITTING_DEVICES + 1];
} waiting_list_t;

bool _should_insert_waitting_list(kburnDeviceNode *node);
void _recreate_waitting_list(KBCTX scope);
waiting_list_t *waiting_list_init();
DECALRE_DISPOSE_HEADER(waiting_list_deinit, waiting_list_t);
