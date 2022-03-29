#pragma once

#include "base.h"
#include "canaan-burn/canaan-burn.h"

typedef struct queue_info *queue_t;
kburn_err_t queue_create(queue_t *queue);
void queue_destroy(queue_t queue);
kburn_err_t queue_push(queue_t queue, void *data);
void *queue_shift(queue_t queue);
