#include <libusb-1.0/libusb.h>

#define usb_interface interface
#define uchar2uint(buf) (((buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3])

#define DEFAULT_VID 0x0559
#define DEFAULT_PID 0x4001

#define BLOCK_SIZE 512			   /* 一个块的大小 */
#define DATA_SIZE 1024 * 1024 * 10 /* 传输一次数据量的大小 */

// Mass Storage Requests values. See section 3 of the Bulk-Only Mass Storage Class specifications
#define BOMS_RESET 0xFF
#define BOMS_GET_MAX_LUN 0xFE
