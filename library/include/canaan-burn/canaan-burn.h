#pragma once

#if BURN_LIB_COMPILING
#define PUBLIC EXPORT
#else
#define PUBLIC IMPORT
#endif

#if defined(_MSC_VER)
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else
#pragma error Unknown dynamic link import / export semantics.
#endif

#include <stdbool.h>
#include <stdint.h>

/* VALUE FROM sercomm */
#define SER_DEV_PATH_SZ 128U

#include "./types.h"

#ifdef __cplusplus
extern "C"
{
#endif

	PUBLIC void kburnSetSerialBaudrate(uint32_t baudrate);
	PUBLIC void kburnSetSerialByteSize(enum KBurnSerialConfigByteSize byteSize);
	PUBLIC void kburnSetSerialParity(enum KBurnSerialConfigParity parity);
	PUBLIC void kburnSetSerialStopBits(enum KBurnSerialConfigStopBits stopBits);
	PUBLIC void kburnSetSerialReadTimeout(int32_t readTimeout);	  // 0 is infinity
	PUBLIC void kburnSetSerialWriteTimeout(int32_t writeTimeout); // 0 is infinity

	/**
	 * 程序结束时，应调用此函数回收资源
	 */
	PUBLIC void kburnDispose(void);

	/**
	 * 直接打开端口，通常不应该调用这个函数
	 * @param path 端口名称（路径）
	 */
	PUBLIC kburnSerialNode *kburnOpen(const char *path);

	typedef bool (*on_device_connect)(const struct kburnSerialNode *dev);
	typedef void (*on_device_handle)(const struct kburnSerialNode *dev);

	/**
	 * 开始监视串口设备，重新运行只设置两个回调
	 * @param verify_callback 新设备连接回调，当返回false时，不试图连接（忽略该端口），该函数可为NULL
	 * @param handler_callback 当设备确认为k510后，用户程序在此处理
	 **/
	PUBLIC void kburnWaitDevice(on_device_connect verify_callback, on_device_handle handler_callback);
	PUBLIC void kburnWaitDevicePause();
	PUBLIC void kburnWaitDeviceResume();

	/**
	 * 当端口断开时，调用此回调，用户可以进行一些清理，该回调执行完后，参数的设备结构会被释放
	 * 只允许注册一个回调，反复调用只有最后一次生效
	 * @param disconnect_callback
	 **/
	PUBLIC void kburnRegisterDisconnectCallback(on_device_handle disconnect_callback);

	/** 调试：获取打开的端口数 */
	PUBLIC uint32_t kburnGetResourceCount();
	/** 调试：获取需要删除的资源数 */
	PUBLIC uint32_t kburnGetOpenPortCount();

#ifdef __cplusplus
}
#endif
