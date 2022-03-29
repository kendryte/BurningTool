#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "./parts/prefix.h"
#include "./parts/errors.h"
#include "./parts/types.h"
#include "./parts/context.h"

DEFINE_START

/**
 * 初始化模块需要的内存
 *
 * @example
 * KBCTX context = NULL;
 *
 * int main() {
 * 	if(kburnCreate(&context) != KBurnNoErr) {
 * 		printf("failed init kendryte burn library!!");
 * 		return 1
 * 	}
 * 	// do something with `context`
 * 	return 0;
 * }
 */
kburn_err_t kburnCreate(struct kburnContext **ppCtx);

/**
 * 程序结束时，应调用此函数回收资源
 */
PUBLIC void kburnGlobalDestroy(void);

/**
 * 回收部分资源
 */
PUBLIC void kburnDestroy(KBCTX scope);

/**
 * 直接打开端口，通常不应该调用这个函数
 * @param path 端口名称（路径）
 */
PUBLIC kburnDeviceNode *kburnOpenSerial(KBCTX scope, const char *path);

/**
 * 新设备连接回调，当返回false时，不试图连接（忽略该端口）
 */
PUBLIC void kburnOnSerialConnect(KBCTX scope, on_device_connect verify_callback, void *ctx);

/**
 * 当设备确认为k510后，用户程序在此处理
 */
PUBLIC void kburnOnSerialConfirm(KBCTX scope, on_device_handle handler_callback, void *ctx);

/**
 * 当端口断开时，调用此回调，用户可以进行一些清理，该回调执行完后，参数的设备结构会被释放
 **/
PUBLIC void kburnOnSerialDisconnect(KBCTX scope, on_device_remove disconnect_callback, void *ctx);

/**
 * 开始监视串口+USB设备
 * 等于两个分别调用
 **/
PUBLIC kburn_err_t kburnStartWaitingDevices(KBCTX scope);
/**
 * 开始监视USB设备
 **/
PUBLIC kburn_err_t kburnStartWaitingUsbDevices(KBCTX scope);
/**
 * 开始监视串口设备
 **/
PUBLIC kburn_err_t kburnStartWaitingSerialDevices(KBCTX scope);

PUBLIC void kburnWaitDevicePause(KBCTX scope);
PUBLIC kburn_err_t kburnWaitDeviceResume(KBCTX scope);

/** 调试：获取需要删除的资源数 */
PUBLIC uint32_t kburnGetResourceCount();
/** 调试：获取打开的端口数 */
PUBLIC uint32_t kburnGetOpenPortCount();

#define KBURN_VIDPID_FILTER_ANY -1
#define KBURN_VIDPID_FILTER_DEFAULT -2
/**
 * 设置usb监视器（和扫描）要响应的设备vid pid
 */
PUBLIC void kburnSetUsbFilter(KBCTX scope, int32_t vid, int32_t pid);

/**
 * 直接打开端口，通常不应该调用这个函数
 * @param path 端口设备路径，最多7层
 */
PUBLIC kburn_err_t kburnOpenUSB(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *path);

/**
 * 单次扫描usb设备
 */
PUBLIC kburn_err_t kburnPollUSB(KBCTX scope);

DEFINE_END

#include "./parts/serial.config.h"
#include "./parts/serial.isp.h"

#ifdef KBURN_ASSERT_SIZE
#include "./parts/static-assert.h"
#endif
