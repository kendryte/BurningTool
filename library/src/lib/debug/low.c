#include "assert.h"
#include "basic/string.h"
#include "user-callback.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#define DEBUG_BUFFER_SIZE 4096

char debug_buffer[DEBUG_BUFFER_SIZE + 1] = "";
char *debug_output = NULL;
size_t debug_buffer_remain;

char *debug_output_move(size_t size) {
	debug_buffer_remain -= size;
	debug_output += size;
	return debug_output;
}

void debug_printf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	debug_output_move(m_vsnprintf(debug_output, debug_buffer_remain, fmt, va));
	va_end(va);
}

void debug_vprintf(const char *fmt, va_list va) {
	debug_output_move(m_vsnprintf(debug_output, debug_buffer_remain, fmt, va));
}

void debug_puts(const char *message) {
	debug_output_move(m_snprintf(debug_output, debug_buffer_remain, "%s", message));
}

pthread_spinlock_t lock = 0;

__attribute__((constructor)) void lock_constructor() {
	m_assert(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE) == 0, "lock destroy fail");
}

__attribute__((destructor)) void lock_destructor() {
	m_assert(pthread_spin_destroy(&lock) == 0, "lock destroy fail");
}

void _kb__debug_enter() {
	pthread_spin_lock(&lock);
	debug_output = debug_buffer;
	debug_buffer_remain = DEBUG_BUFFER_SIZE;
	debug_buffer[0] = '\0';
}

void _kb__debug_leave(kburnLogType type) {
	assert(debug_output - debug_buffer <= DEBUG_BUFFER_SIZE);
	assert(debug_output[0] == '\0');
	debug_callback_call(type, debug_buffer);
	pthread_spin_unlock(&lock);
}
