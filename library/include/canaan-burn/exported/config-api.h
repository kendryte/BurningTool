#pragma once

#include "./prefix.h"
#include "./types.serial.h"

DEFINE_START

#define KBURN_K510_BAUDRATE_DEFAULT 115200
PUBLIC void kburnSetSerialFastbootBaudrate(KBCTX scope, uint32_t baudrate);
PUBLIC uint32_t kburnGetSerialFastbootBaudrate(KBCTX scope);

PUBLIC void kburnSetSerialByteSize(KBCTX scope, enum KBurnSerialConfigByteSize byteSize);
PUBLIC enum KBurnSerialConfigByteSize kburnGetSerialByteSize(KBCTX scope);

PUBLIC void kburnSetSerialParity(KBCTX scope, enum KBurnSerialConfigParity parity);
PUBLIC enum KBurnSerialConfigParity kburnGetSerialParity(KBCTX scope);

PUBLIC void kburnSetSerialStopBits(KBCTX scope, enum KBurnSerialConfigStopBits stopBits);
PUBLIC enum KBurnSerialConfigStopBits kburnGetSerialStopBits(KBCTX scope);

PUBLIC void kburnSetSerialReadTimeout(KBCTX scope, int32_t readTimeout); // 0 is infinity
PUBLIC int32_t kburnGetSerialReadTimeout(KBCTX scope);

PUBLIC void kburnSetSerialWriteTimeout(KBCTX scope, int32_t writeTimeout); // 0 is infinity
PUBLIC int32_t kburnGetSerialWriteTimeout(KBCTX scope);

PUBLIC void kburnSetSerialBaudrate(KBCTX scope, uint32_t baudrate);
PUBLIC uint32_t kburnGetSerialBaudrate(KBCTX scope);

PUBLIC void kburnSetSerialFailRetry(KBCTX scope, uint8_t retryTimes);
PUBLIC uint8_t kburnGetSerialFailRetry(KBCTX scope);

#define KBURN_K510_BAUDRATE_USBISP 115200
PUBLIC void kburnSetSerialUsbIspBaudrate(KBCTX scope, uint32_t baudrate);
PUBLIC uint32_t kburnGetSerialUsbIspBaudrate(KBCTX scope);

#define KBURN_USB_DEFAULT_VID 0x0559
#define KBURN_USB_DEFAULT_PID 0x4001
PUBLIC void kburnSetUsbFilterVid(KBCTX scope, uint16_t vid);
PUBLIC uint16_t kburnGetUsbFilterVid(KBCTX scope);

PUBLIC void kburnSetUsbFilterPid(KBCTX scope, uint16_t pid);
PUBLIC uint16_t kburnGetUsbFilterPid(KBCTX scope);

DEFINE_END
