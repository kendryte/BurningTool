#pragma once

#include <stdio.h>
#include <string.h>

#include "base.h"
#include "basic/lock.h"
#include "context.h"
#include "canaan-burn/canaan-burn.h"
#include "device.h"
#include "basic/disposable.h"

#include "basic/sleep.h"

char *sprintf_alloc(const char *fmt, ...);
char *vsprintf_alloc(const char *fmt, va_list args);

static inline const char *NULLSTR(const char *str)
{
	return str ? str : "<NULLSTR>";
}

static inline bool
prefix(const char *pre, const char *str)
{
	return strncmp(pre, str, strlen(pre)) == 0;
}

#include "basic/macros.error.h"

#include "basic/macros.alloc.h"
#include "basic/debug-print.h"
#include "basic/resource-tracker.h"
