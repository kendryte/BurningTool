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
	disposable_registry *dis = KBALLOC(disposable_registry);
	FREE_WITH(dis, dis);

	*ppCtx = KBALLOC_SCOPE(dis, kburnContext);

	snprintf(dis->comment, sizeof(dis->comment), "context 0x%p", (void *)*ppCtx);

	kburnContext src = {
		.serial = KBALLOC_SCOPE(dis, serial_subsystem_context),
		.usb = KBALLOC_SCOPE(dis, usb_subsystem_context),
		.disposables = dis,
		.openDeviceList = KBALLOC_SCOPE(dis, struct port_link_list),
		.monitor_inited = false,
	};
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
