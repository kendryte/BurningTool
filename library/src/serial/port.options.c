#include "serial.h"

ser_opts_t opts = SER_OPTS_INIT;

int32_t serial_open_with_current_config(ser_t *ser, const char *path)
{
	ser_opts_t _opts = SER_OPTS_INIT;
	memcpy(&_opts, &opts, sizeof(ser_opts_t));
	_opts.port = path;

	return ser_open(ser, &_opts);
}

void kburnSetSerialBaudrate(uint32_t baudrate)
{
	opts.baudrate = baudrate;
}
void kburnSetSerialByteSize(enum KBurnSerialConfigByteSize byteSize)
{
	switch (byteSize)
	{
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
		assert("byteSize is invalid");
	}
}
void kburnSetSerialParity(enum KBurnSerialConfigParity parity)
{
	switch (parity)
	{
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
		assert("parity is invalid");
	}
}
void kburnSetSerialStopBits(enum KBurnSerialConfigStopBits stopBits)
{
	switch (stopBits)
	{
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
		assert("stopBits is invalid");
	}
}
void kburnSetSerialReadTimeout(int32_t readTimeout)
{
	opts.timeouts.rd = readTimeout;
}
void kburnSetSerialWriteTimeout(int32_t writeTimeout)
{
	opts.timeouts.wr = writeTimeout;
}
