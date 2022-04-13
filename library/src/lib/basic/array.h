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
void array_move(dynamic_array_t *dst, dynamic_array_t *src);
bool array_fit(dynamic_array_t *array, uint32_t new_size);
void p_array_push(dynamic_array_t *array, void *new_element);
void p_array_remove(dynamic_array_t *array, size_t index);
ssize_t p_array_find(dynamic_array_t *array, void *needle);
#define array_create(element_type, n) _kb_array_create(sizeof(element_type), n)
#define array_make(variable, element_type, n) _kb_array_make(&variable, sizeof(element_type), n)
DECALRE_DISPOSE_HEADER(array_destroy, dynamic_array_t);
#define array_grow(array, n) array_fit(array, array->size + n);
