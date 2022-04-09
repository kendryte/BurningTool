#pragma once

#include "./prefix.h"
#include "./types.serial.h"

DEFINE_START

PUBLIC void kburnSetSerialBaudrate(uint32_t baudrate);
PUBLIC void kburnSetSerialByteSize(enum KBurnSerialConfigByteSize byteSize);
PUBLIC void kburnSetSerialParity(enum KBurnSerialConfigParity parity);
PUBLIC void kburnSetSerialStopBits(enum KBurnSerialConfigStopBits stopBits);
PUBLIC void kburnSetSerialReadTimeout(int32_t readTimeout);   // 0 is infinity
PUBLIC void kburnSetSerialWriteTimeout(int32_t writeTimeout); // 0 is infinity

#define KBURN_K510_BAUDRATE_DEFAULT 115200
#define KBURN_K510_BAUDRATE_USBISP 115200
PUBLIC void kburnSetSerialHighSpeedBaudrate(uint32_t baudrate);
PUBLIC uint32_t kburnGetSerialHighSpeedBaudrate();

DEFINE_END
