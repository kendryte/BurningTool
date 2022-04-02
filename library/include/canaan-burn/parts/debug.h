#pragma once

#include "./prefix.h"
#include "./types.h"

DEFINE_START

struct kburnDebugColor
{
	const char *prefix;
	const char *postfix;
};
typedef struct kburnDebugColors
{
	struct kburnDebugColor red;
	struct kburnDebugColor green;
	struct kburnDebugColor yellow;
	struct kburnDebugColor grey;
} kburnDebugColors;
PUBLIC on_debug_log kburnSetLogCallback(on_debug_log callback);
PUBLIC void kburnSetLogBufferEnabled(bool enable);
PUBLIC kburnDebugColors kburnSetColors(kburnDebugColors color_list);

DEFINE_END
