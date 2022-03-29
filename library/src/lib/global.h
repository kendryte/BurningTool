#pragma once

#include <stdio.h>
#include <string.h>

#include "base.h"
#include "context.h"
#include "canaan-burn/canaan-burn.h"
#include "device.h"
#include "basic/lock.h"
#include "basic/disposable.h"

#include "basic/sleep.h"

char *sprintf_alloc(const char *fmt, ...);

static inline const char *NULLSTR(const char *str)
{
	return str ? str : "<NULLSTR>";
}

static inline bool
prefix(const char *pre, const char *str)
{
	return strncmp(pre, str, strlen(pre)) == 0;
}

#define IfErrorReturn(action) __extension__({ \
	kburn_err_t _err = action;                \
	if (_err != KBurnNoErr)                   \
		return _err;                          \
	_err;                                     \
})

#include "basic/alloc.h"
#include "basic/endian.h"
#include "basic/debug-print.h"
#include "basic/resource-tracker.h"
