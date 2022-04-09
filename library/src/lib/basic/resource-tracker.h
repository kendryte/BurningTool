#pragma once

#include "disposable.h"
#include "debug/path.h"

typedef void (*cleanup_function)(void *value);

typedef struct_debug_bundle resource_tracker_debug;
typedef void (*handle_function)(void *dev, void *ctx);

#define MAX_TRACE 20
typedef struct resource_tracker_element {
	cleanup_function callback;
	handle_function user_callback;
	void *user_callback_ctx;
	void *resource;
	disposable dispose_object;
	bool force;

	resource_tracker_debug __debug;
} resource_tracker_element;
typedef struct resource_tracker {
	bool successed;
	bool hasAlways;
	resource_tracker_element element[MAX_TRACE];
	uint8_t size;

	resource_tracker_debug __debug;
} resource_tracker_t;

_Static_assert(MAX_TRACE < UINT8_MAX, "max trace may not too large");

void resource_tracker_done(resource_tracker_t *tracker);
void keep_resource(resource_tracker_t *tracker);
void *_track_resource(resource_tracker_t *tracker, void *resource, cleanup_function clean, bool always, resource_tracker_debug dbg);
void *_track_dispose(resource_tracker_t *tracker, disposable d, resource_tracker_debug dbg);

#define resource_tracker_t_init                   \
	(resource_tracker_t){                         \
		.successed = false,                       \
		.hasAlways = false,                       \
		.size = false,                            \
		.__debug = DEBUG_SAVE("ResourceTracker"), \
	};

#define DeferEnabled resource_tracker_t __attribute__((cleanup(resource_tracker_done))) _resource_tracker = resource_tracker_t_init

#define DeferCall(cleaner, pointer)                                                                                                    \
	__extension__({                                                                                                                    \
		if (0)                                                                                                                         \
			cleaner(pointer);                                                                                                          \
		_track_resource(&_resource_tracker, (void *)pointer, (cleanup_function)cleaner, false, DEBUG_SAVE(#cleaner "(" #pointer ")")); \
	})
#define DeferCallAlways(cleaner, pointer)                                                                                             \
	__extension__({                                                                                                                   \
		if (0)                                                                                                                        \
			cleaner(pointer);                                                                                                         \
		_track_resource(&_resource_tracker, (void *)pointer, (cleanup_function)cleaner, true, DEBUG_SAVE(#cleaner "(" #pointer ")")); \
	})
#define DeferFree(pointer) DeferCall(free, pointer)
#define DeferFreeAlways(pointer) DeferCallAlways(free, pointer)
#define DeferDispose(list, obj, destruct)                                                                  \
	__extension__({                                                                                        \
		disposable d = toDisposable(destruct, obj);                                                        \
		_track_dispose(&_resource_tracker, dispose_list_add(list, d), DEBUG_SAVE(#destruct "(" #obj ")")); \
	})

#define DeferAbort keep_resource(&_resource_tracker)
