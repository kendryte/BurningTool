#pragma once

typedef int referece_counter_t;
#define reference_increase(obj) (void)__atomic_add_fetch(&obj, 1, __ATOMIC_RELAXED)
#define reference_decrease(obj) __atomic_sub_fetch(&obj, 1, __ATOMIC_RELAXED) == 0
