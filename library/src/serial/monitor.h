#pragma once

#include "context.h"

kburn_err_t serial_monitor_prepare(KBCTX scope);
void serial_monitor_destroy(KBCTX scope);
void serial_monitor_pause(KBCTX scope);
kburn_err_t serial_monitor_resume(KBCTX scope);
