#pragma once

#include "base.h"
#include "disposable.h"

typedef struct dynamic_array {
	uint32_t size;
	uint32_t length;
	size_t element_size;
	bool free_self;
	void *body;
} dynamic_array_t;

dynamic_array_t *_kb_array_create(size_t element_size, uint32_t init_size);
bool _kb_array_make(dynamic_array_t *array, size_t element_size, uint32_t init_size);
void array_delete(dynamic_array_t *array);
bool array_resize(dynamic_array_t *array, uint32_t new_size);
bool array_fit(dynamic_array_t *array, uint32_t new_size);
#define array_create(element_type, n) _kb_array_create(sizeof(element_type), n)
#define array_make(variable, element_type, n) _kb_array_make(&variable, sizeof(element_type), n)
DECALRE_DISPOSE_HEADER(array_destroy, dynamic_array_t);
