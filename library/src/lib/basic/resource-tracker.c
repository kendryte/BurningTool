#include "resource-tracker.h"
#include "debug/print.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

static inline void do_cleanup(const resource_tracker_element element) {
	if (element.resource != NULL) {
		element.callback(element.resource);
	} else if (element.dispose_object.list != NULL) {
		dispose(element.dispose_object);
	} else {
		m_abort("invalid cleanup");
	}
}

void resource_tracker_done(resource_tracker_t *tracker) {
	if (!tracker->successed) {
		debug_print_dbg(KBURN_LOG_WARN, tracker->__debug, "function return without confirm, release all resource: %s(%d)",
						DEBUG_OBJ_TITLE(tracker->__debug), tracker->size);
		for (int i = tracker->size - 1; i >= 0; i--) {
			debug_print_dbg(KBURN_LOG_WARN, tracker->element[i].__debug, " * %s", DEBUG_OBJ_TITLE(tracker->element[i].__debug));
			do_cleanup(tracker->element[i]);
		}
	} else if (tracker->hasAlways) {
		for (int i = tracker->size - 1; i >= 0; i--) {
			if (tracker->element[i].force) {
				debug_print_dbg(KBURN_LOG_INFO, tracker->element[i].__debug, " * [force] %s", DEBUG_OBJ_TITLE(tracker->element[i].__debug));
				do_cleanup(tracker->element[i]);
			}
		}
	}

	memset(tracker, 0, sizeof(resource_tracker_t));
}
void keep_resource(resource_tracker_t *tracker) { tracker->successed = true; }
void *_track_resource(resource_tracker_t *tracker, void *resource, cleanup_function clean, bool always, resource_tracker_debug dbg) {
	m_assert_ptr(resource, "can not track null ptr");
	uint8_t i = tracker->size++;
	m_assert(i < MAX_TRACE, "resource tracker can not hold too many element");
	memset(tracker->element + i, 0, sizeof(resource_tracker_element));
	tracker->element[i].callback = clean;
	tracker->element[i].resource = resource;
	tracker->element[i].force = always;
	tracker->element[i].__debug = dbg;
	if (always)
		tracker->hasAlways = true;

	return resource;
}

void *_track_dispose(resource_tracker_t *tracker, disposable d, resource_tracker_debug dbg) {
	m_assert_ptr(d.list, "can not track disposable without add it to list");
	uint8_t i = tracker->size++;
	m_assert(i < MAX_TRACE, "resource tracker can not hold too many element");
	memset(tracker->element + i, 0, sizeof(resource_tracker_element));
	tracker->element[i].dispose_object = d;
	tracker->element[i].force = false;
	tracker->element[i].__debug = dbg;

	return d.object;
}
