#define has_kb_mutex 1

#include <pthread.h>

#define THREAD_TITLE_BUFF_SIZE 32
typedef struct kb_mutex {
	pthread_mutex_t *mutex;
	char holder[THREAD_TITLE_BUFF_SIZE];
	const char *varname;
	const char *file;
	int line;
} kb_mutex;

#include "sleep.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "debug/print.h"

pthread_mutex_t *_kb_init_lock() {
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
void _kb_deinit_lock(pthread_mutex_t **mutex) {
	debug_print(KBURN_LOG_TRACE, "[lock] deinit");
	int err = pthread_mutex_destroy(*mutex);
	if (err != 0) {
		debug_print(KBURN_LOG_ERROR, COLOR_FMT("failed pthread_mutex_destroy: %d: %s"), COLOR_ARG(YELLOW, err, strerror(err)));
	}
	free(*mutex);
	*mutex = NULL;
}

void _kb_lock(pthread_mutex_t *mutex) {
	// debug_print(KBURN_LOG_TRACE, "  lock", (void *)mutex, sizeof(pthread_mutex_t));
	m_assert0(pthread_mutex_lock(mutex), "pthread_mutex_lock failed");
}

void _kb_unlock(pthread_mutex_t *mutex) {
	// debug_print(KBURN_LOG_TRACE, "unlock", (void *)mutex, sizeof(pthread_mutex_t));
	m_assert0(pthread_mutex_unlock(mutex), "pthread_mutex_unlock failed");
}

kb_mutex *_kb__init_lock() {
	kb_mutex *mlock = calloc(1, sizeof(kb_mutex));
	mlock->mutex = _kb_init_lock();
	return mlock;
}

void _kb__deinit_lock(kb_mutex **pmlock) {
	kb_mutex *mlock = *pmlock;
	*pmlock = NULL;

	pthread_mutex_destroy(mlock->mutex);
	free(mlock->mutex);
	free(mlock);
}

static inline void _dbg_ln(const char *msg, const char *varname, const char *RELEASE_UNUSED(file), int RELEASE_UNUSED(line)) {
	debug_print(KBURN_LOG_ERROR, COLOR_FMT("%s"), COLOR_ARG(RED, msg));
	char buff[THREAD_TITLE_BUFF_SIZE] = {0};
	pthread_getname_np(pthread_self(), buff, THREAD_TITLE_BUFF_SIZE);
	debug_print_location(KBURN_LOG_ERROR, file, line, "[thread: %s] try lock - %s", buff, varname);
}

void _kb__lock(kb_mutex *mlock, const char *varname, const char *file, int line) {
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

		_kb_lock(mlock->mutex);
	} else if (re != 0) {
		_dbg_ln(strerror(re), varname, file, line);
		m_abort("lock error");
	}

	mlock->file = file;
	mlock->line = line;
	mlock->varname = varname;
	pthread_getname_np(pthread_self(), mlock->holder, THREAD_TITLE_BUFF_SIZE);
}

void _kb__unlock(kb_mutex *mlock) {
	// debug_print("unlock %p", (void *)mlock->mutex);
	_kb_unlock(mlock->mutex);
	mlock->file = NULL;
	mlock->line = -1;
	mlock->holder[0] = '\0';
	mlock->varname = NULL;
}

pthread_mutex_t *_kb__raw_lock(kb_mutex *mlock) {
	return mlock->mutex;
}
