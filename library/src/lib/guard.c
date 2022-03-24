#include "serial.h"
#include "usb.h"

static_assert(sizeof(KBURN_ERROR_KIND_USB) == sizeof(uint64_t), "error kind must be 64bit");
static_assert(sizeof(enum kburnErrorKind) == sizeof(uint64_t), "error kind must be 64bit");
static_assert(sizeof(kburnIspOperation) == 1, "enum must 8bit");
static_assert(sizeof(kburnIspErrorCode) == 1, "enum must 8bit");
