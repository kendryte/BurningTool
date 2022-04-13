#include "lock.h"
#include "array.h"
#include "sleep.h"
#include <errno.h>
#include <pthread.h>
#include "debug/print.h"

struct mlock {
	referece_counter_t waitting;
	pthread_mutex_t mutex;

	reference_destroy destruct;
	void *context;
	void *object;
	bool mark_delete;

#ifndef NDEBUG
	debug_bundle _dbg;
#endif
};

kb_mutex_t lock_init() {
	kb_mutex_t mlock = calloc(1, sizeof(struct mlock));
	if (mlock == NULL) {
		debug_print(KBURN_LOG_ERROR, "  ! memory error");
		return NULL;
	}

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK); // PTHREAD_MUTEX_RECURSIVE
	pthread_mutex_init(&mlock->mutex, &attr);

	return mlock;
}

void _lock_bind_destruct(kb_mutex_t mlock, reference_destroy destruct, void *context, void *object) {
	m_assert(mlock->destruct == NULL, "duplicate call to _lock_register_destructor");
	autolock(mlock);
	mlock->destruct = destruct;
	mlock->object = object;
	mlock->context = context;
	mlock->mark_delete = false;
}

void lock_deinit(kb_mutex_t mlock) {
	if (mlock->mark_delete) {
		return;
	}

	lock(mlock);

	mlock->mark_delete = true;

	unlock(mlock);
	if (lock(mlock)) {
		unlock(mlock);
	}
}

static bool atomic_check_and_destroy(kb_mutex_t mlock) {
	bool last = reference_decrease(mlock->waitting);
	if (mlock->mark_delete) {
		if (last) {
			pthread_mutex_unlock(&mlock->mutex);
			int err = pthread_mutex_destroy(&mlock->mutex);
			if (err != 0) {
				debug_print(KBURN_LOG_ERROR, COLOR_FMT("failed pthread_mutex_destroy: %d: %s"), COLOR_ARG(YELLOW, err, strerror(err)));
			}

			if (mlock->destruct) {
				mlock->destruct(mlock->context, mlock->object);
			}

			free(mlock);
		}
		return false;
	}

	return true;
}

#ifndef NDEBUG
bool _kb__lock(kb_mutex_t mlock, debug_bundle dbg) {
#else
bool _kb__lock(kb_mutex_t mlock) {
#endif
	reference_increase(mlock->waitting);
	m_assert0(pthread_mutex_lock(&mlock->mutex), "pthread_mutex_lock failed");
	if (!atomic_check_and_destroy(mlock)) {
		return false;
	}

#ifndef NDEBUG
	mlock->_dbg = dbg;
#endif

	return true;
}

void unlock(kb_mutex_t mlock) {
#ifndef NDEBUG
	memset(&mlock->_dbg, 0, sizeof(struct debug_bundle));
#endif
	m_assert0(pthread_mutex_unlock(&mlock->mutex), "pthread_mutex_unlock failed");
}

pthread_mutex_t *raw_lock(kb_mutex_t mlock) {
	return &mlock->mutex;
}
