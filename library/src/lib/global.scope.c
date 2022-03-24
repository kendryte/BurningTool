#include "global.h"
#include "usb.h"
#include "serial.h"
#include "device-link-list.h"

disposable_registry lib_global_scope = {
	.comment = "entire global scope",
	.head = NULL,
	.tail = NULL,
	.lock = 0,
	.size = 0,
};

kburn_err_t kburnCreate(KBCTX *ppCtx)
{
	static uint32_t dbg_index = 0;
	disposable_registry *dis = KBALLOC(disposable_registry);
	register_dispose_pointer(dis, dis);

	dis->comment = sprintf_alloc("context %d", dbg_index++);
	if (dis->comment)
		register_dispose_pointer_null(dis, dis->comment);

	*ppCtx = CALLOC_AUTO_FREE(dis, kburnContext);

	kburnContext src = {
		.serial = CALLOC_AUTO_FREE(dis, serial_subsystem_context),
		.usb = CALLOC_AUTO_FREE(dis, usb_subsystem_context),
		.disposables = dis,
		.openDeviceList = CALLOC_AUTO_FREE(dis, struct port_link_list),
		.monitor_inited = false,
	};

	src.usb->filter.vid = DEFAULT_VID;
	src.usb->filter.pid = DEFAULT_PID;

	memcpy(*ppCtx, &src, sizeof(kburnContext));

	dispose_chain(&lib_global_scope, dis);

	return KBurnNoErr;
}

void kburnGlobalDestroy()
{
	debug_print("kburnGlobalDestroy()");
	dispose(&lib_global_scope);
}

void kburnDestroy(KBCTX scope)
{
	dispose_child(&lib_global_scope, scope->disposables);
}

uint32_t kburnGetResourceCount()
{
	uint32_t i = 0;
	disposable_foreach_start(&lib_global_scope, item);
	const KBCTX scope = (KBCTX)item->userData;
	i += scope->disposables->size;
	disposable_foreach_end(&lib_global_scope);
	return i;
}

uint32_t kburnGetOpenPortCount()
{
	uint32_t i = 0;
	disposable_foreach_start(&lib_global_scope, item);
	const KBCTX scope = (KBCTX)item->userData;
	i += scope->openDeviceList->size;
	disposable_foreach_end(&lib_global_scope);
	return i;
}
