#include "event-queue.h"
#include "basic/resource-tracker.h"

typedef struct queue_struct {
	void *data;
	struct queue_struct *next;
} queue_struct;

typedef struct queue_info {
	queue_struct *first;
	queue_struct *last;
	size_t length;
} queue_info;

kburn_err_t queue_create(queue_info **queue_ptr) {
	DeferEnabled;

	*queue_ptr = MyAlloc(queue_info);
	DeferCall(free, queue_ptr);

	DeferAbort;
	return KBurnNoErr;
}

void *queue_shift(queue_info *queue) {
	if (queue->first == NULL) {
		return NULL;
	}
	queue_struct *old = queue->first;

	if (queue->last == queue->first) {
		queue->last = NULL;
	}
	queue->first = old->next;

	queue->length--;

	void *ret = old->data;
	free(old);
	return ret;
}

size_t queue_size(queue_info *queue) {
	return queue->length;
}

void queue_destroy(queue_info *queue, pointer_handler element_handle) {
	if (queue == NULL) {
		return;
	}

	void *ele;
	while ((ele = queue_shift(queue)) != NULL) {
		if (element_handle != NULL) {
			element_handle(ele);
		}
	}

	free(queue);
}

kburn_err_t queue_push(queue_info *queue, void *data) {
	queue_struct *new = MyAlloc(queue_struct);
	new->data = data;

	if (queue->first == NULL) {
		queue->first = new;
		queue->last = new;
	} else {
		queue->last->next = new;
		queue->last = new;
	}

	queue->length++;

	return KBurnNoErr;
}
