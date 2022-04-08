#include "base.h"
#include "context.h"
#include "basic/errors.h"
#include "private-types.h"
#include "components/thread.h"
#include <windows.h>
#include <dbt.h>
#include <usbiodef.h>
#include "debug/print.h"

typedef struct polling_context {
	kbthread thread;
	HWND window;
	bool notify;
} polling_context;

static void OnDeviceChange(UINT nEventType, DEV_BROADCAST_HDR *dwData) {
	// https://docs.microsoft.com/en-us/windows/win32/devio/wm-devicechange
	if (nEventType != DBT_DEVNODES_CHANGED)
		return;

	debug_print(LOG_);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DEVICECHANGE:
		OnDeviceChange(wParam, (void *)lParam);
		break;
	default:
		// debug_print(KBURN_LOG_TRACE, "WM: %d", msg);
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

static HWND create_event_window() {
	// http://www.winprog.org/tutorial/simple_window.html

	WNDCLASSEX wc;

	static const char *windowClass = "usbEventReceiver";

	// Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;
	// wc.hInstance = GetModuleHandleA(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = windowClass;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		debug_print_win32("Window RegisterClassEx Failed!");
		return NULL;
	}
	debug_print(KBURN_LOG_DEBUG, "Window RegisterClassEx OK");

	// Step 2: Creating the Window
	HWND hwnd = CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_STATICEDGE, windowClass, "usb event handle window", WS_VISIBLE,
							   CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, NULL, NULL, NULL, NULL);

	if (hwnd == NULL) {
		debug_print_win32("Window CreateWindowEx Failed!");
		return NULL;
	}

	debug_print(KBURN_LOG_DEBUG, "Window CreateWindowEx OK");

	return hwnd;
}

static void usb_monitor_polling_thread(void *UNUSED(context), KBCTX scope, const bool *const quit) {
	HWND hwnd = create_event_window();
	if (!hwnd) {
		m_abort("create window failed");
	}
	scope->usb->polling_context->window = hwnd;

	// register event
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

	HDEVNOTIFY hDevNotify = RegisterDeviceNotification(scope->usb->polling_context->window, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if (NULL == hDevNotify) {
		debug_print_win32("Window RegisterDeviceNotification Failed!");
	}
	debug_print(KBURN_LOG_DEBUG, "Window RegisterDeviceNotification OK");

	// #ifndef NDEBUG
	// 	if (!ShowWindow(hwnd, SW_SHOWNORMAL))
	// #else
	if (!ShowWindow(hwnd, SW_HIDE))
	// #endif
	{
		debug_print_win32("Window ShowWindow Failed!");
	}

	if (!UpdateWindow(hwnd)) {
		debug_print_win32("Window UpdateWindow Failed!");
	}

	// Step 3: The Message Loop
	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0) > 0 && !*quit) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	if (!UnregisterDeviceNotification(hDevNotify)) {
		debug_print_win32("Window UnregisterDeviceNotification Failed!");
	}

	if (!CloseWindow(hwnd)) {
		debug_print_win32("Window CloseWindow Failed!");
	}

	if (!DestroyWindow(hwnd)) {
		debug_print_win32("Window DestroyWindow Failed!");
	}
}

kburn_err_t usb_monitor_polling_prepare(KBCTX scope) {
	if (!scope->usb->polling_context)
		scope->usb->polling_context = MyAlloc(struct polling_context);

	IfErrorReturn(thread_create("usb poll", usb_monitor_polling_thread, NULL, scope, &scope->usb->polling_context->thread));

	return KBurnNoErr;
}

void usb_monitor_polling_destroy(KBCTX scope) {
	if (!scope->usb->polling_context)
		return;

	if (scope->usb->polling_context->window) {
		PostMessageA(scope->usb->polling_context->window, WM_CLOSE, 0, 0);
	}

	if (scope->usb->polling_context->thread) {
		thread_destroy(scope, scope->usb->polling_context->thread);
		scope->usb->polling_context->thread = NULL;
	}

	free(scope->usb->polling_context);
	scope->usb->polling_context = NULL;
}

void usb_monitor_polling_pause(KBCTX scope) { scope->usb->polling_context->notify = false; }

kburn_err_t usb_monitor_polling_resume(KBCTX scope) {
	scope->usb->polling_context->notify = true;
	return KBurnNoErr;
}
