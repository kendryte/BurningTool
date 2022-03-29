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
	kb_mutex_t mutex;
} queue_info;

kburn_err_t queue_create(queue_info **queue_ptr)
{
	DeferEnabled;

	*queue_ptr = MyAlloc(queue_info);
	DeferCall(free, queue_ptr);

	(*queue_ptr)->mutex = CheckNull(lock_init());

	DeferAbort;
	return KBurnNoErr;
}

static void *_queue_shift(queue_info *queue)
{
	if (queue->first == NULL)
	{
		return NULL;
	}
	queue_struct *old = queue->first;

	if (queue->last == queue->first)
		queue->last = NULL;
	queue->first = old->next;

	return old->data;
}

void queue_destroy(queue_info *queue)
{
	lock(queue->mutex);
	void *ele;
	while ((ele = _queue_shift(queue)) != NULL)
	{
		free(ele);
	}
	kb_mutex_t mutex = queue->mutex;
	free(queue);
	unlock(mutex);
	lock_deinit(&mutex);
}

kburn_err_t queue_push(queue_info *queue, void *data)
{
	queue_struct *new = MyAlloc(queue_struct);
	new->data = data;

	lock(queue->mutex);
	// TODO: 当此处刚好与queue_destroy同时运行时，queue已经被释放，下面的读写都会造成崩溃
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
	unlock(queue->mutex);

	return KBurnNoErr;
}

void *queue_shift(queue_info *queue)
{
	autolock(queue->mutex);
	return _queue_shift(queue);
}
