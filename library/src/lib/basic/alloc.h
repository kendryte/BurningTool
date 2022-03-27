#pragma once

#define CheckNull(ptr) __extension__({  \
	void *kb_pointer_validate = (ptr); \
	if (kb_pointer_validate == NULL)   \
		return KBurnNoMemory;          \
	kb_pointer_validate;               \
})

#define __MyAlloc_2(type, cnt) (type *)__extension__({ \
	CheckNull(calloc(cnt, sizeof(type)));               \
})
#define __MyAlloc_1(type) __MyAlloc_2(type, 1)
#define __MyAlloc_X(A0, A1, A2, FN, ...) FN

#define MyAlloc(...) __MyAlloc_X(, ##__VA_ARGS__,          \
								 __MyAlloc_2(__VA_ARGS__), \
								 __MyAlloc_1(__VA_ARGS__))
