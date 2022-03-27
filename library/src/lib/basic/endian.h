#pragma once

#ifdef __LITTLE_ENDIAN__
#include <byteswap.h>
#define CHIP_ENDIAN32(x) bswap_32(x)
#else
#define CHIP_ENDIAN32(x) x
#endif
