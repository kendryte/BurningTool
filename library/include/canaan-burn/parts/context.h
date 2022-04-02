#pragma once

#include "./prefix.h"

#if BURN_LIB_COMPILING
#pragma error "this should not include"
#endif

typedef struct kburnContext kburnContext;
typedef kburnContext *KBCTX;
