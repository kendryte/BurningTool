#pragma once

#include "disposable.h"

typedef void (*cleanup_function)(void *value);

typedef struct debug_bundle resource_tracker_debug;

#define MAX_TRACE 20
typedef struct resource_tracker_element
{
	cleanup_function callback;
	void *resource;
	disposable dispose_object;
	bool force;

	resource_tracker_debug __debug;
} resource_tracker_element;
typedef struct resource_tracker
{
	bool successed;
	bool hasAlways;
	resource_tracker_element element[MAX_TRACE];
	uint8_t size;

	resource_tracker_debug __debug;
} resource_tracker_t;

_Static_assert(MAX_TRACE < UINT8_MAX, "max trace may not too large");

void resource_tracker_done(resource_tracker_t *tracker);
void keep_resource(resource_tracker_t *tracker);
void *_track_resource(resource_tracker_t *tracker, void *resource, cleanup_function clean, bool always, const char *var, const char *file, int line);
void *_track_dispose(resource_tracker_t *tracker, disposable d, const char *var, const char *file, int line);

#define resource_tracker_t_init (resource_tracker_t){ \
	.successed = false,                               \
	.hasAlways = false,                               \
	.size = false,                                    \
	.__debug = {                                      \
		.file = __FILENAME__,                         \
		.func = __func__,                             \
		.line = __LINE__,                             \
		.title = "ResourceTracker",                   \
	},                                                \
};

#define DeferEnabled resource_tracker_t __attribute__((cleanup(resource_tracker_done))) _resource_tracker = resource_tracker_t_init

#define DeferCall(cleaner, pointer) __extension__({                                                                           \
	if (0)                                                                                                                    \
		cleaner(pointer);                                                                                                     \
	_track_resource(&_resource_tracker, (void *)pointer, (cleanup_function)cleaner, false, #pointer, __FILENAME__, __LINE__); \
})
#define DeferFree(pointer) DeferCall(free, pointer)
#define DeferDispose(list, obj, destruct) __extension__({                                                          \
	disposable d = toDisposable(destruct, obj);                                                                    \
	_track_dispose(&_resource_tracker, dispose_list_add(list, d), #destruct "(" #obj ")", __FILENAME__, __LINE__); \
})

#define DeferAbort keep_resource(&_resource_tracker)