#include "global.h"

typedef struct queue_struct
{
	void *data;
	struct queue_struct *next;
	bool free_when_destroy;
} queue_struct;

typedef struct queue_info
{
	queue_struct *first;
	queue_struct *last;
	volatile int lock;
} queue_info;

kburn_err_t queue_create(queue_info **queue_ptr)
{
	*queue_ptr = KBALLOC(queue_info);
	return KBurnNoErr;
}

static void *_queue_shift(queue_info *queue, bool do_free)
{
	lock(&queue->lock);
	if (queue->first == NULL)
	{
		unlock(&queue->lock);
		return NULL;
	}
	queue_struct *old = queue->first;

	if (queue->last == queue->first)
		queue->last = NULL;
	queue->first = old->next;

	unlock(&queue->lock);

	void *data = old->data;
	free(old);

	if (do_free)
	{
		free(data);
		return NULL;
	}

	return data;
}

void queue_destroy(queue_info *queue)
{
	while (_queue_shift(queue, true) != NULL)
		;
	free(queue);
}

kburn_err_t queue_push(queue_info *queue, void *data, bool free_when_destroy)
{
	queue_struct *new = KBALLOC(queue_struct);
	new->data = data;
	new->free_when_destroy = free_when_destroy;

	lock(&queue->lock);
	if (queue->first == NULL)
	{
		queue->first = new;
		queue->last = new;
	}
	else
	{
		queue->last->next = new;
		queue->last = new;
	}
	unlock(&queue->lock);

	return KBurnNoErr;
}

void *queue_shift(queue_info *queue)
{
	return _queue_shift(queue, false);
}
