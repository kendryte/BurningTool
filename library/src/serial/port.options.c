#include "port.options.h"
#include "context.h"
#include "canaan-burn/exported/serial.config.h"
#include "canaan-burn/exported/types.serial.h"
#include <sercomm/sercomm.h>
#include "debug/print.h"

ser_opts_t opts = {
	.port = NULL,
	.baudrate = KBURN_K510_BAUDRATE_DEFAULT,
	.bytesz = SER_BYTESZ_8,
	.parity = SER_PAR_NONE,
	.stopbits = SER_STOPB_ONE,
	.timeouts = {.rd = 1000, .wr = 1000},
};

ser_opts_t get_current_serial_options(const char *path) {
	ser_opts_t _opts = SER_OPTS_INIT;
	memcpy(&_opts, &opts, sizeof(ser_opts_t));
	_opts.port = path;
	return _opts;
}

void kburnSetSerialBaudrate(uint32_t baudrate) {
	debug_trace_function("%d", baudrate);
	opts.baudrate = baudrate;
}
void kburnSetSerialByteSize(enum KBurnSerialConfigByteSize byteSize) {
	debug_trace_function("%d", byteSize);
	switch (byteSize) {
	case KBurnSerialConfigByteSize_8:
		opts.bytesz = SER_BYTESZ_8;
		break;
	case KBurnSerialConfigByteSize_7:
		opts.bytesz = SER_BYTESZ_7;
		break;
	case KBurnSerialConfigByteSize_6:
		opts.bytesz = SER_BYTESZ_6;
		break;
	case KBurnSerialConfigByteSize_5:
		opts.bytesz = SER_BYTESZ_5;
		break;
	default:
		m_abort("byteSize is invalid");
	}
}
void kburnSetSerialParity(enum KBurnSerialConfigParity parity) {
	debug_trace_function("%d", parity);
	switch (parity) {
	case KBurnSerialConfigParityNone:
		opts.parity = SER_PAR_NONE;
		break;
	case KBurnSerialConfigParityOdd:
		opts.parity = SER_PAR_ODD;
		break;
	case KBurnSerialConfigParityEven:
		opts.parity = SER_PAR_EVEN;
		break;
	case KBurnSerialConfigParityMark:
		opts.parity = SER_PAR_MARK;
		break;
	case KBurnSerialConfigParitySpace:
		opts.parity = SER_PAR_SPACE;
		break;

	default:
		m_abort("parity is invalid");
	}
}
void kburnSetSerialStopBits(enum KBurnSerialConfigStopBits stopBits) {
	debug_trace_function("%d", stopBits);
	switch (stopBits) {
	case KBurnSerialConfigStopBitsOne:
		opts.stopbits = SER_STOPB_ONE;
		break;
	case KBurnSerialConfigStopBitsOneHalf:
		opts.stopbits = SER_STOPB_ONE5;
		break;
	case KBurnSerialConfigStopBitsTwo:
		opts.stopbits = SER_STOPB_TWO;
		break;
	default:
		m_abort("stopBits is invalid");
	}
}
void kburnSetSerialReadTimeout(int32_t readTimeout) {
	debug_trace_function("%d", readTimeout);
	opts.timeouts.rd = readTimeout;
}
void kburnSetSerialWriteTimeout(int32_t writeTimeout) {
	debug_trace_function("%d", writeTimeout);
	opts.timeouts.wr = writeTimeout;
}
