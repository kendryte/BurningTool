#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "resource-tracker.h"
#include "debug-print.h"

static inline void do_cleanup(const resource_tracker_element element)
{
	if (element.resource != NULL)
	{
		element.callback(element.resource);
	}
	else if (element.dispose_object.list != NULL)
	{
		dispose(element.dispose_object);
	}
	else
	{
		m_abort("invalid cleanup");
	}
}

void resource_tracker_done(resource_tracker_t *tracker)
{
	if (!tracker->successed)
	{
		debug_print("function return without confirm, release all resource: (%d)", tracker->size);
		for (uint8_t i = 0; i < tracker->size; i++)
		{
			debug_print_location(tracker->element[i].file, tracker->element[i].line, " * %s", tracker->element[i].var);
			do_cleanup(tracker->element[i]);
		}
	}
	else if (tracker->hasAlways)
	{
		for (uint8_t i = 0; i < tracker->size; i++)
		{
			if (tracker->element[i].force)
			{
				debug_print_location(tracker->element[i].file, tracker->element[i].line, "delete resource [force] %s", tracker->element[i].var);
				tracker->element[i].callback(*(void **)tracker->element[i].resource);
				do_cleanup(tracker->element[i]);
			}
		}
	}

	memset(tracker, 0, sizeof(resource_tracker_t));
}
void keep_resource(resource_tracker_t *tracker)
{
	tracker->successed = true;
}
void *_track_resource(resource_tracker_t *tracker, void *resource, cleanup_function clean, bool always, const char *var, const char *file, int line)
{
	m_assert_ptr(resource, "can not track null ptr");
	uint8_t i = tracker->size++;
	m_assert(i < MAX_TRACE, "resource tracker can not hold too many element");
	tracker->element[i].callback = clean;
	tracker->element[i].resource = resource;
	tracker->element[i].force = always;
	tracker->element[i].file = file;
	tracker->element[i].line = line;
	tracker->element[i].var = var;
	if (always)
		tracker->hasAlways = true;

	return resource;
}

void *_track_dispose(resource_tracker_t *tracker, disposable d, const char *var, const char *file, int line)
{
	m_assert_ptr(d.list, "can not track disposable without add it to list");
	uint8_t i = tracker->size++;
	m_assert(i < MAX_TRACE, "resource tracker can not hold too many element");
	tracker->element[i].callback = NULL;
	tracker->element[i].resource = NULL;
	tracker->element[i].dispose_object = d;
	tracker->element[i].force = false;
	tracker->element[i].file = file;
	tracker->element[i].line = line;
	tracker->element[i].var = var;

	return d.object;
}
