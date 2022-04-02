#pragma once

#include <stdint.h>

enum kburnIspOperation {
	ISP_ECHO = 0xC1,
	ISP_NOP = 0xC2,
	ISP_MEMORY_WRITE = 0xC3,
	ISP_MEMORY_READ = 0xC4,
	ISP_MEMORY_BOOT = 0xC5,
	ISP_UARTHS_BAUDRATE_SET = 0xC6,
	ISP_DEBUG_INFO = 0xD1,
	ISP_FLASH_ERASE = 0xD3,
	ISP_FLASH_WRITE = 0xD4,
	ISP_REBOOT = 0xD5,
	ISP_FLASH_INIT = 0xD7,
	ISP_FLASH_READ = 0xD8,
	ISP_OTP_READ = 0xD9,
	ISP_OTP_WRITE = 0xDA,
	ISP_EMMC_READ = 0xE0,
	ISP_EMMC_WRITE = 0xE1,
	ISP_GET_CHIPID = 0xF0,

	ISP_OP_MAX = UINT8_MAX,
} __attribute__((__packed__));

typedef enum kburnIspOperation kburnIspOperation;
_Static_assert(sizeof(kburnIspOperation) == 1, "enum must 8bit");
