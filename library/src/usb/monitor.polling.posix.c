#include "base.h"
#include "context.h"
#include "private-types.h"
#include "components/thread.h"

typedef struct polling_context {
	kbthread thread;
} polling_context;

kburn_err_t usb_monitor_polling_prepare(KBCTX UNUSED(scope)) {
	TODO;
	return KBurnNoErr;
}
void usb_monitor_polling_destroy(KBCTX UNUSED(scope)) {}
void usb_monitor_polling_pause(KBCTX UNUSED(scope)) {}
kburn_err_t usb_monitor_polling_resume(KBCTX UNUSED(scope)) { return KBurnNoErr; }
