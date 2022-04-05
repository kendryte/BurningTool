#pragma once

#include "context.h"

kburn_err_t usb_monitor_prepare(KBCTX scope);
void usb_monitor_destroy(KBCTX scope);
void usb_monitor_pause(KBCTX scope);
kburn_err_t usb_monitor_resume(KBCTX scope);
