#include "components/thread.h"
#include "private-types.h"
#include "protocol.h"

kburn_err_t serial_subsystem_init(KBCTX scope) {
	debug_trace_function();
	if (scope->serial->subsystem_inited)
		return KBurnNoErr;

	kburn_err_t r = thread_create("pairing", pair_serial_ports_thread, NULL, scope, &scope->serial->pairing_thread);
	if (r != KBurnNoErr)
		return r;

	scope->serial->subsystem_inited = true;
	return KBurnNoErr;
}
void serial_subsystem_deinit(KBCTX scope) {
	debug_trace_function();
	if (!scope->serial->subsystem_inited)
		return;
	scope->serial->subsystem_inited = false;
}
