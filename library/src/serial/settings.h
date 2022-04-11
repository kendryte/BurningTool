#pragma once

#include "context.h"
#include <sercomm/sercomm.h>

ser_opts_t get_current_serial_options(KBCTX scope, const char *path);
