#pragma once

// checker: char (*__)[sizeof(xxx)] = 1;
_Static_assert(sizeof(kburnErrorDesc) == sizeof(uint64_t) + sizeof(int32_t), "kburnErrorDesc must be 96bit");
_Static_assert(sizeof(KBURN_ERROR_KIND_USB) == sizeof(uint64_t), "error kind must be 64bit");
_Static_assert(sizeof(enum kburnErrorKind) == sizeof(uint64_t), "error kind must be 64bit");
_Static_assert(sizeof(kburnIspErrorCode) == 1, "enum must 8bit");
