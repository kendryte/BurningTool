#pragma once

#include "base.h"
#include "canaan-burn/canaan-burn.h"

typedef void (*pointer_handler)(void *data);
typedef struct queue_info *queue_t;
kburn_err_t queue_create(queue_t *queue);
void queue_destroy(queue_t queue, pointer_handler element_handle);
kburn_err_t queue_push(queue_t queue, void *data);
void *queue_shift(queue_t queue);
size_t queue_size(queue_t queue);
