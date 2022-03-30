#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef UNUSED
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_##x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#else
#define UNUSED(x)
#endif

#define CONCAT(a, b) a##b

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x

#define __VaridicMacro_Helper1(A0, A1, FN, ...) FN
#define __VaridicMacro_Helper2(A0, A1, A2, FN, ...) FN
#define __VaridicMacro_Helper3(A0, A1, A2, A3, FN, ...) FN
#define __VaridicMacro_Helper4(A0, A1, A2, A3, A4, FN, ...) FN
#define __VaridicMacro_Helper5(A0, A1, A2, A3, A4, A5, FN, ...) FN
