#include <errno.h>
#include <fcntl.h>
#include <serial.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/serial.h>
#endif

#if defined(__MACH__) && defined(__APPLE__)
#include <AvailabilityMacros.h>
#include <IOKit/serial/ioss.h>
#ifndef TIOCINQ
#define TIOCINQ FIONREAD
#endif
#endif

// https://github.com/ingeniamc/sercomm/blob/master/include/sercomm/posix/types.h
struct ser {
	struct termios tios_old;
	int fd;
	struct {
		int rd;
		int wr;
	} timeouts;
};

// https://github.com/ingeniamc/sercomm/blob/master/sercomm/posix/comms.c#L123
static speed_t convert_speed(uint32_t br) {
	switch (br) {
#ifdef B0
	case 0:
		return B0;
#endif
#ifdef B50
	case 50:
		return B50;
#endif
#ifdef B75
	case 75:
		return B75;
#endif
#ifdef B110
	case 110:
		return B110;
#endif
#ifdef B134
	case 134:
		return B134;
#endif
#ifdef B150
	case 150:
		return B150;
#endif
#ifdef B200
	case 200:
		return B200;
#endif
#ifdef B300
	case 300:
		return B300;
#endif
#ifdef B600
	case 600:
		return B600;
#endif
#ifdef B1200
	case 1200:
		return B1200;
#endif
#ifdef B1800
	case 1800:
		return B1800;
#endif
#ifdef B2400
	case 2400:
		return B2400;
#endif
#ifdef B4800
	case 4800:
		return B4800;
#endif
#ifdef B7200
	case 7200:
		return B7200;
#endif
#ifdef B9600
	case 9600:
		return B9600;
#endif
#ifdef B14400
	case 14400:
		return B14400;
#endif
#ifdef B19200
	case 19200:
		return B19200;
#endif
#ifdef B28800
	case 28800:
		return B28800;
#endif
#ifdef B57600
	case 57600:
		return B57600;
#endif
#ifdef B76800
	case 76800:
		return B76800;
#endif
#ifdef B38400
	case 38400:
		return B38400;
#endif
#ifdef B115200
	case 115200:
		return B115200;
#endif
#ifdef B128000
	case 128000:
		return B128000;
#endif
#ifdef B153600
	case 153600:
		return B153600;
#endif
#ifdef B230400
	case 230400:
		return B230400;
#endif
#ifdef B256000
	case 256000:
		return B256000;
#endif
#ifdef B460800
	case 460800:
		return B460800;
#endif
#ifdef B576000
	case 576000:
		return B576000;
#endif
#ifdef B921600
	case 921600:
		return B921600;
#endif
#ifdef B1000000
	case 1000000:
		return B1000000;
#endif
#ifdef B1152000
	case 1152000:
		return B1152000;
#endif
#ifdef B1500000
	case 1500000:
		return B1500000;
#endif
#ifdef B2000000
	case 2000000:
		return B2000000;
#endif
#ifdef B2500000
	case 2500000:
		return B2500000;
#endif
#ifdef B3000000
	case 3000000:
		return B3000000;
#endif
#ifdef B3500000
	case 3500000:
		return B3500000;
#endif
#ifdef B4000000
	case 4000000:
		return B4000000;
#endif
	default:
		return __MAX_BAUD + 1;
	}
}

static bool set_speed(kburnSerialDeviceNode *node, int fd, uint32_t br, struct termios *tios) {
	speed_t speed = convert_speed(br);

	if (speed <= __MAX_BAUD) {
		cfsetispeed(tios, speed);
		cfsetospeed(tios, speed);

		if (tcsetattr(fd, TCSANOW, tios) != 0) {
			set_syserr(node);
			return false;
		}
	} else {
#if defined(__MACH__) && defined(__APPLE__)
		/* Mac OS X (>= 10.4) */
		if (ioctl(fd, IOSSIOSPEED, (speed_t *)&br, 1) < 0) {
			node->error->code = make_error_code(KBURN_ERROR_KIND_SYSCALL, errno);
			set_error(node, strerror(errno));
			return false;
		}
#elif defined(__linux__)
		/* Linux */
		struct serial_struct lser;

		if (ioctl(fd, TIOCGSERIAL, &lser) < 0) {
			set_syserr(node);
			return false;
		}

		/* set custom divisor and update flags */
		lser.custom_divisor = lser.baud_base / (int)br;
		lser.flags &= ~ASYNC_SPD_MASK;
		lser.flags |= ASYNC_SPD_CUST;

		if (ioctl(fd, TIOCSSERIAL, &lser) < 0) {
			set_syserr(node);
			return false;
		}
#else
		set_error(node, KBURN_ERROR_KIND_COMMON, KBurnSerialDriverUnsupportChange, "Custom baudrates unsupported");
		return false;
#endif
	}

	return true;
}

bool hackdev_serial_low_switch_baudrate(kburnSerialDeviceNode *node, uint32_t speed) {
	debug_trace_function("node[%s], %d", node->path, speed);
	autolock(node->mutex);
	struct ser *ser = node->m_dev_handle;
	struct termios new_termios;
	if (tcgetattr(ser->fd, &new_termios) < 0) {
		set_error(node, KBURN_ERROR_KIND_COMMON, KBurnSerialDriverAttrReadErr, "failed read driver attr");
		unlock(node->mutex);
		return false;
	}

	if (!set_speed(node, ser->fd, speed, &new_termios)) {
		unlock(node->mutex);
		return false;
	}

	serial_low_flush_all(node);
	do_sleep(500);
	serial_low_drain_input(node);

	node->baudRate = speed;
	return true;
}
