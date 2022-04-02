#pragma once

#include "base.h"

#define CheckNull(ptr)                                                                                                                               \
	__extension__({                                                                                                                                  \
		void *kb_pointer_validate = (ptr);                                                                                                           \
		if (kb_pointer_validate == NULL)                                                                                                             \
			return KBurnNoMemory;                                                                                                                    \
		kb_pointer_validate;                                                                                                                         \
	})

#define __MyAlloc_2(type, cnt) (type *)__extension__({ CheckNull(calloc(cnt, sizeof(type))); })
#define __MyAlloc_1(type) __MyAlloc_2(type, 1)

#define MyAlloc(...) __VaridicMacro_Helper2(, ##__VA_ARGS__, __MyAlloc_2(__VA_ARGS__), __MyAlloc_1(__VA_ARGS__))
