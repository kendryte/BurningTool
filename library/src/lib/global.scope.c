#include <pthread.h>
#include "global.h"
#include "usb.h"
#include "serial.h"
#include "components/device-link-list.h"
#include "bind-wait-list.h"

disposable_list_t *lib_global_scope;

kburn_err_t kburnCreate(KBCTX *ppCtx)
{
	DeferEnabled;

	static uint32_t dbg_index = 0;
	char *comment = DeferFree(CheckNull(sprintf_alloc("context %d", dbg_index++)));

	disposable_list_t *dis = DeferFree(CheckNull(disposable_list_init(comment)));
	DeferCall(dispose_all, dis);

	register_dispose_pointer(dis, comment);

	*ppCtx = MyAlloc(kburnContext);
	register_dispose_pointer(dis, *ppCtx);

	waiting_list_t *wlist = CheckNull(waiting_list_init());
	dispose_list_add(dis, toDisposable(waiting_list_deinit, wlist));

	serial_subsystem_context *serial = MyAlloc(serial_subsystem_context);
	register_dispose_pointer(dis, serial);

	usb_subsystem_context *usb = MyAlloc(usb_subsystem_context);
	register_dispose_pointer(dis, usb);
	usb->filter.vid = DEFAULT_VID;
	usb->filter.pid = DEFAULT_PID;

	struct port_link_list *odlist = port_link_list_init();
	dispose_list_add(dis, toDisposable(port_link_list_deinit, odlist));

	memcpy(*ppCtx, &(kburnContext){
					   .serial = serial,
					   .usb = usb,
					   .openDeviceList = odlist,
					   .waittingDevice = wlist,
					   .disposables = dis,
					   .monitor_inited = false,
				   },
		   sizeof(kburnContext));

	if (!lib_global_scope)
		lib_global_scope = disposable_list_init("library global");

	dispose_list_add(lib_global_scope, toDisposable(dispose_all_and_deinit, dis));

	DeferAbort;
	return 0;
}

void kburnGlobalDestroy()
{
	debug_print("kburnGlobalDestroy()");
	dispose_all(lib_global_scope);
	disposable_list_deinit(lib_global_scope);
}

void kburnDestroy(KBCTX scope)
{
	dispose(bindToList(lib_global_scope, toDisposable(dispose_all_and_deinit, scope->disposables)));
}
