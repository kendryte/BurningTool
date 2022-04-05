#pragma once

#include "./prefix.h"

// checker: char (*__)[sizeof(xxx)] = 1;
_Static_assert(sizeof(kburnErrorDesc) == sizeof(uint64_t) + sizeof(int32_t), "struct kburnErrorDesc must be 96bit");
_Static_assert(sizeof(enum kburnErrorKind) == sizeof(uint64_t), "enum kburnErrorKind must be 64bit");
_Static_assert(sizeof(kburnIspErrorCode) == 1, "enum kburnIspErrorCode must 8bit");
