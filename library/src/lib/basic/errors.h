#pragma once

#include "context.h"
#include "errors.h"
#include <stdint.h>

kburn_err_t make_error_code(enum kburnErrorKind kind, int32_t code);
