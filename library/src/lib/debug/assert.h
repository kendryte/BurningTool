#pragma once

_Noreturn void m_assert_print_abort(const char *ncondition_str, const char *file, int line, const char *message, ...);
#define m_abort(fmt, ...) m_assert_print_abort("abort()", __FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__)
#define m_assert(ncondition, fmt, ...)                                                                                                               \
	if (!(ncondition)) {                                                                                                                             \
		m_assert_print_abort(#ncondition, __FILE__, __LINE__, fmt __VA_OPT__(, ) __VA_ARGS__);                                                       \
	}

#define m_assert0(value, exmsg)                                                                                                                      \
	__extension__({                                                                                                                                  \
		bool va = (value) == 0;                                                                                                                      \
		m_assert(va, #value " == 0", "value of %s is %d (except 0) %s", #value, va, exmsg);                                                          \
	})

#define m_assert_ptr(ptr, fmt, ...)                                                                                                                  \
	__extension__({                                                                                                                                  \
		void *p = ptr;                                                                                                                               \
		m_assert(p != NULL, fmt __VA_OPT__(, ) __VA_ARGS__);                                                                                         \
	})

#define m_assert_err(value, exmsg)                                                                                                                   \
	__extension__({                                                                                                                                  \
		kburnErrorDesc e = kburnSplitErrorCode(value);                                                                                               \
		m_assert(e.kind == 0 && e.code == 0, "Error [kind=%d, code=%d]: %s", e.kind >> 32, e.code, exmsg);                                           \
	})
