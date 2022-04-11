#include "context.h"
#include "../bridge/bind-wait-list.h"
#include "../serial/private-types.h"
#include "../usb/private-types.h"
#include "basic/resource-tracker.h"
#include "components/call-user-handler.h"
#include "components/device-link-list.h"
#include <pthread.h>

disposable_list_t *lib_global_scope = NULL;
static uint32_t dbg_index = 0;
extern struct serial_settings serial_default_settings;
extern struct usb_settings usb_default_settings;

void kburnGlobalDestroy() {
	debug_trace_function();
	if (lib_global_scope == NULL) {
		return;
	}
	dispose_all(lib_global_scope);
	disposable_list_deinit(lib_global_scope);
	lib_global_scope = NULL;
}

kburn_err_t kburnCreate(KBCTX *ppCtx) {
	debug_trace_function();

	DeferEnabled;

	dbg_index++;

	char *comment1 = DeferFree(CheckNull(sprintf_alloc("scope[%d].disposables", dbg_index)));
	disposable_list_t *dis = DeferFree(CheckNull(disposable_list_init(comment1)));
	DeferCall(dispose_all, dis);

	char *comment2 = DeferFree(CheckNull(sprintf_alloc("scope[%d].threads", dbg_index)));
	disposable_list_t *threads = DeferFree(CheckNull(disposable_list_init(comment2)));
	DeferCall(dispose_all, threads);

	register_dispose_pointer(dis, comment1);
	register_dispose_pointer(dis, comment2);

	*ppCtx = MyAlloc(kburnContext);
	register_dispose_pointer(dis, *ppCtx);

	waiting_list_t *wlist = CheckNull(waiting_list_init());
	dispose_list_add(dis, toDisposable(waiting_list_deinit, wlist));

	serial_subsystem_context *serial = MyAlloc(serial_subsystem_context);
	register_dispose_pointer(dis, serial);

	serial->settings = serial_default_settings;

	usb_subsystem_context *usb = MyAlloc(usb_subsystem_context);
	register_dispose_pointer(dis, usb);
	usb->settings = usb_default_settings;

	struct port_link_list *odlist = port_link_list_init();
	dispose_list_add(dis, toDisposable(port_link_list_deinit, odlist));

	memcpy(
		*ppCtx,
		&(kburnContext){
			.signature = CONTEXT_MEMORY_SIGNATURE,
			.serial = serial,
			.usb = usb,
			.openDeviceList = odlist,
			.waittingDevice = wlist,
			.disposables = dis,
			.threads = threads,
			.monitor_inited = false,
		},
		sizeof(kburnContext));

	if (!lib_global_scope) {
		atexit(kburnGlobalDestroy);
		lib_global_scope = disposable_list_init("library global");
	}

	dispose_list_add(lib_global_scope, toDisposable(dispose_all_and_deinit, dis));
	dispose_list_add(lib_global_scope, toDisposable(dispose_all_and_deinit, threads));

	IfErrorReturn(global_init_user_handle_thread(*ppCtx));

	DeferAbort;
	return KBurnNoErr;
}

void kburnDestroy(KBCTX scope) {
	debug_trace_function("(%p)", (void *)scope);
	if (lib_global_scope == NULL) {
		return;
	}
	dispose(bindToList(lib_global_scope, toDisposable(dispose_all_and_deinit, scope->threads)));
	dispose(bindToList(lib_global_scope, toDisposable(dispose_all_and_deinit, scope->disposables)));
}
