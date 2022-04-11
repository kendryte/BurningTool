#include "settings.h"
#include "context.h"
#include "private-types.h"
#include "canaan-burn/canaan-burn.h"
#include <sercomm/sercomm.h>
#include "debug/print.h"

const struct serial_settings serial_default_settings = {
	.retry_times = 3,
	.command_baudrate = 115200,
	.isp_baudrate = 115200,
	.transfer_baudrate = 460800,
	.byte_size = KBurnSerialConfigByteSize_8,
	.parity = KBurnSerialConfigParityNone,
	.stop_bits = KBurnSerialConfigStopBitsOne,
	.read_timeout = 1000,
	.write_timeout = 1000,
};

static ser_bytesz_t bytesize_map_to_lib(enum KBurnSerialConfigByteSize byteSize) {
	switch (byteSize) {
	case KBurnSerialConfigByteSize_8:
		return SER_BYTESZ_8;
	case KBurnSerialConfigByteSize_7:
		return SER_BYTESZ_7;
	case KBurnSerialConfigByteSize_6:
		return SER_BYTESZ_6;
	case KBurnSerialConfigByteSize_5:
		return SER_BYTESZ_5;
	default:
		m_abort("byteSize is invalid");
	}
}
static ser_parity_t parity_map_to_lib(enum KBurnSerialConfigParity parity) {
	switch (parity) {
	case KBurnSerialConfigParityNone:
		return SER_PAR_NONE;
	case KBurnSerialConfigParityOdd:
		return SER_PAR_ODD;
	case KBurnSerialConfigParityEven:
		return SER_PAR_EVEN;
	case KBurnSerialConfigParityMark:
		return SER_PAR_MARK;
	case KBurnSerialConfigParitySpace:
		return SER_PAR_SPACE;
	default:
		m_abort("parity is invalid");
	}
}
static ser_stopbits_t stopbits_map_to_lib(enum KBurnSerialConfigStopBits stopBits) {
	switch (stopBits) {
	case KBurnSerialConfigStopBitsOne:
		return SER_STOPB_ONE;
	case KBurnSerialConfigStopBitsOneHalf:
		return SER_STOPB_ONE5;
	case KBurnSerialConfigStopBitsTwo:
		return SER_STOPB_TWO;
	default:
		m_abort("stopBits is invalid");
	}
}

ser_opts_t get_current_serial_options(KBCTX scope, const char *path) {
	return (ser_opts_t){
		.port = path,
		.baudrate = subsystem_settings.command_baudrate,
		.bytesz = bytesize_map_to_lib(subsystem_settings.byte_size),
		.parity = parity_map_to_lib(subsystem_settings.parity),
		.stopbits = stopbits_map_to_lib(subsystem_settings.stop_bits),
		.timeouts.rd = subsystem_settings.read_timeout,
		.timeouts.wr = subsystem_settings.write_timeout,
	};
}

CREATE_GETTER_SETTER(Baudrate, transfer_baudrate)

CREATE_GETTER_SETTER(FastbootBaudrate, command_baudrate)

CREATE_GETTER_SETTER(UsbIspBaudrate, transfer_baudrate)

CREATE_GETTER_SETTER(ByteSize, byte_size)

CREATE_GETTER_SETTER(Parity, parity)

CREATE_GETTER_SETTER(StopBits, stop_bits)

CREATE_GETTER_SETTER(ReadTimeout, read_timeout)

CREATE_GETTER_SETTER(WriteTimeout, write_timeout)

CREATE_GETTER_SETTER(FailRetry, retry_times)
