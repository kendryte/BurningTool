#include "array.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "debug/print.h"

dynamic_array_t *_kb_array_create(size_t element_size, uint32_t init_size) {
	dynamic_array_t *array = malloc(sizeof(dynamic_array_t));
	if (array == NULL)
		return array;

	_kb_array_make(array, element_size, init_size);

	array->free_self = true;
	return array;
}
bool _kb_array_make(dynamic_array_t *array, size_t element_size, uint32_t init_size) {
	memset(array, 0, sizeof(dynamic_array_t));
	array->element_size = element_size;
	return array_resize(array, init_size);
}
void array_delete(dynamic_array_t *array) {
	if (array == NULL)
		return;

	free(array->body);
	if (array->free_self)
		free(array);
}
bool array_resize(dynamic_array_t *array, uint32_t new_size) {
	void *ret = realloc(array->body, new_size * array->element_size);

	if (ret == NULL)
		return false;

	array->body = ret;
	array->size = new_size;

	if (array->length < array->size) {
		size_t start = array->length * array->element_size;
		size_t total = array->size * array->element_size;
		memset((char *)array->body + start, 0, total - start);
	}

	return true;
}
bool array_fit(dynamic_array_t *array, uint32_t new_size) {
	if (array->size < new_size) {
		return array_resize(array, new_size);
	} else {
		return true;
	}
}
DECALRE_DISPOSE(array_destroy, dynamic_array_t) { array_delete(context); }
DECALRE_DISPOSE_END()
