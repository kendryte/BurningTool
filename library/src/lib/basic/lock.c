#define has_kb_mutex 1
#include "sleep.h"
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define THREAD_TITLE_BUFF_SIZE 32

typedef struct kb_mutex {
	pthread_mutex_t *mutex;
	char holder[THREAD_TITLE_BUFF_SIZE];
	const char *varname;
	const char *file;
	int line;
} kb_mutex_t;

#include "debug/print.h"

pthread_mutex_t *_init_lock() {
	// debug_print("[lock] init");
	pthread_mutexattr_t attr;
	pthread_mutex_t *ret = calloc(1, sizeof(pthread_mutex_t));
	if (ret == NULL) {
		debug_print(KBURN_LOG_ERROR, "  ! memory error");
		return NULL;
	}
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init(ret, &attr);
	return ret;
}
void _deinit_lock(pthread_mutex_t **mutex) {
	debug_print(KBURN_LOG_TRACE, "[lock] deinit");
	m_assert0(pthread_mutex_destroy(*mutex), "unlock fail");
	free(*mutex);
	*mutex = NULL;
}

void _lock(pthread_mutex_t *mutex) {
	// debug_print(KBURN_LOG_TRACE, "  lock", (void *)mutex, sizeof(pthread_mutex_t));
	m_assert0(pthread_mutex_lock(mutex), "pthread_mutex_lock failed");
}

void _unlock(pthread_mutex_t *mutex) {
	// debug_print(KBURN_LOG_TRACE, "unlock", (void *)mutex, sizeof(pthread_mutex_t));
	m_assert0(pthread_mutex_unlock(mutex), "pthread_mutex_unlock failed");
}

kb_mutex_t *__init_lock() {
	kb_mutex_t *mlock = calloc(1, sizeof(kb_mutex_t));
	mlock->mutex = _init_lock();
	return mlock;
}

void __deinit_lock(kb_mutex_t **pmlock) {
	kb_mutex_t *mlock = *pmlock;
	*pmlock = NULL;

	pthread_mutex_destroy(mlock->mutex);
	free(mlock->mutex);
	free(mlock);
}

static inline void _dbg_ln(const char *msg, const char *varname, const char *file, int line) {
	debug_print(KBURN_LOG_ERROR, COLOR_FMT("%s"), COLOR_ARG(RED, msg));
	char buff[THREAD_TITLE_BUFF_SIZE] = {0};
	pthread_getname_np(pthread_self(), buff, THREAD_TITLE_BUFF_SIZE);
	debug_print_location(KBURN_LOG_ERROR, file, line, "[thread: %s] try lock - %s", buff, varname);
}

void __lock(kb_mutex_t *mlock, const char *varname, const char *file, int line) {
	if (mlock == NULL) {
		_dbg_ln("lock uninit mutex object", varname, file, line);
		// no return
	}

	if (mlock->mutex == NULL) {
		_dbg_ln("lock destroy mutex object", varname, file, line);
		// no return
	}

	// debug_print("lock %p", (void *)mlock->mutex);
	int re = pthread_mutex_timedlock(mlock->mutex, &(struct timespec){.tv_nsec = 0, .tv_sec = 5});
	if (re == ETIMEDOUT) {
		_dbg_ln("can not aquire lock in 5s", varname, file, line);
		debug_print_location(KBURN_LOG_ERROR, mlock->file, mlock->line, "[thread: %s] hold by %s", mlock->holder, mlock->varname);

		_lock(mlock->mutex);
	} else if (re != 0) {
		_dbg_ln(strerror(re), varname, file, line);
		m_abort("lock error");
	}

	mlock->file = file;
	mlock->line = line;
	mlock->varname = varname;
	pthread_getname_np(pthread_self(), mlock->holder, THREAD_TITLE_BUFF_SIZE);
}

void __unlock(kb_mutex_t *mlock) {
	// debug_print("unlock %p", (void *)mlock->mutex);
	_unlock(mlock->mutex);
	mlock->file = NULL;
	mlock->line = -1;
	mlock->holder[0] = '\0';
	mlock->varname = NULL;
}

pthread_mutex_t *__raw_lock(kb_mutex_t *mlock) { return mlock->mutex; }
