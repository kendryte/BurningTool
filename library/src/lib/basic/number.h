#pragma once

#define MIN(A, B)            \
	__extension__({          \
		__typeof__(A) a = A; \
		__typeof__(B) b = B; \
		__typeof__(A) r = 0; \
		if (a > b)           \
			r = b;           \
		else                 \
			r = a;           \
		r;                   \
	})
