#include "global.h"
#include "usb.h"
#include "serial.h"
#include "device-link-list.h"

disposable_registry lib_global_scope = {NULL, NULL, 0, 0};

static DECALRE_DISPOSE(scope_delete, kburnContext)
{
	debug_print("scope_delete(0x%p)", (void *)context);
	dispose(context->disposables);
	free(context->disposables);
	free(context->usb);
	free(context->serial);
	free(context->openDeviceList);
	free(context);
}
DECALRE_DISPOSE_END()

kburn_err_t kburnCreate(KBCTX *ppCtx)
{
	*ppCtx = KBALLOC(kburnContext);

	kburnContext src = {
		.serial = KBALLOC(serial_subsystem_context),
		.usb = KBALLOC(usb_subsystem_context),
		.disposables = KBALLOC(disposable_registry),
		.openDeviceList = KBALLOC(struct port_link_list),
		.monitor_inited = false,
	};
	memcpy(*ppCtx, &src, sizeof(kburnContext));

	dispose_add(&lib_global_scope, disposable(scope_delete, *ppCtx));

	return KBurnNoErr;
}

void kburnGlobalDestroy()
{
	debug_print("kburnGlobalDestroy()");
	dispose(&lib_global_scope);
}

void kburnDestroy(KBCTX scope)
{
	scope_delete(&lib_global_scope, scope);
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
