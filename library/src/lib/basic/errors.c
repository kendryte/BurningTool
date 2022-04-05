#include "canaan-burn/exported/errors.h"
#include <error.h>
#include <sys/types.h>
typedef union error_convert {
	struct {
#if BYTE_ORDER == LITTLE_ENDIAN
		int32_t code;
		uint32_t kind;
#else
		uint32_t kind;
		int32_t code;
#endif
	} __attribute__((__packed__)) split;
	kburn_err_t combine;
} error_convert;

kburnErrorDesc kburnSplitErrorCode(kburn_err_t code) {
	error_convert c = {.combine = code};

	return (kburnErrorDesc){
		.code = c.split.code,
		.kind = ((uint64_t)c.split.kind) << 32,
	};
}
kburn_err_t make_error_code(enum kburnErrorKind kind, int32_t code) {
	error_convert c = {
		.split = {.kind = kind >> 32, .code = code}
	   };

	return c.combine;
}
