#pragma once

#include "device.h"
#include "lock.h"
#include "canaan-burn/canaan-burn.h"
#include <stdatomic.h>

// typedef void (*on_reference_destroy)(void *ctx, void *object);
// typedef struct referece_collector {
// 	int count;
// 	void *destructor_context;
// 	on_reference_destroy destructor;
// 	bool delete_mark;
// } * referece_collector_t;
// #define REF_COLLECT_T struct referece_collector referece_collector

// int _referece_collector_increase(void *object, referece_collector_t collector);
// #define reference_increase(obj) _Generic((obj), GENERIC_LINE(referece_counter_t, __atomic_add_fetch(&obj, 1, __ATOMIC_RELAXED)),
// GENERIC_LINE(referece_collector_t, _referece_collector_increase(obj, obj->referece_collector)))

// void __Ref_create(Ref obj, void *object, on_reference_destroy destroy, void *destroy_ctx);
// /* Device Node */
// static inline kburnDeviceNode *_node_reference_inc(kburnDeviceNode *node) {
// 	kburnDeviceRef(node);
// 	return node;
// }
// #define node_reference_inc(node) _node_reference_inc(get_node(node))

// static inline kburnDeviceNode *_node_reference_dec(kburnDeviceNode *node) {
// 	kburnDeviceUnRef(node);
// 	return node;
// }
// #define node_reference_dec(node) _node_reference_dec(get_node(node))

// static inline void node_reference_dec_ptr(kburnDeviceNode **node) {
// 	kburnDeviceUnRef(*node);
// }
// #define node_reference_local(node) kburnDeviceNode *__attribute__((cleanup(node_reference_dec_ptr))) nodeTemp =
// _node_reference_inc(get_node(node));

// void _node_reference_mark_delete(kburnDeviceNode *node);
// #define node_reference_mark_delete(node) _node_reference_mark_delete(get_node(node))
