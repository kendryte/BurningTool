#pragma once

#include "./exported/errors.h"
#include "./exported/prefix.h"
#include "./exported/types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if BURN_LIB_COMPILING
#include "context.h"
#else
#include "./exported/context.h"
#endif

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
PUBLIC kburn_err_t kburnCreate(struct kburnContext **ppCtx);

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
PUBLIC kburn_err_t kburnOpenSerial(KBCTX scope, const char *path);

/**
 * 新设备连接回调，当返回false时，不试图连接（忽略该端口）
 */
PUBLIC on_device_connect_t kburnOnSerialConnect(KBCTX scope, on_device_connect verify_callback, void *ctx);

/**
 * 当设备确认为k510后，用户程序在此处理
 */
PUBLIC on_device_handle_t kburnOnSerialConfirm(KBCTX scope, on_device_handle handler_callback, void *ctx);

/**
 * 当端口断开时，调用此回调，用户可以进行一些清理，该回调执行完后，参数的设备结构会被释放
 * 如果断开的设备没有打开过，dev=NULL，此时可以刷新可用端口列表
 **/
PUBLIC on_device_remove_t kburnOnDeviceDisconnect(KBCTX scope, on_device_remove disconnect_callback, void *ctx);

/**
 * 设备列表发生某些改变时调用（比connect、disconnect早）
 */
PUBLIC on_device_list_change_t kburnOnDeviceListChange(KBCTX scope, on_device_list_change change_callback, void *ctx);

/**
 * 新设备连接回调，当返回false时，不试图连接（忽略该端口）
 */
PUBLIC on_device_connect_t kburnOnUsbConnect(KBCTX scope, on_device_connect verify_callback, void *ctx);

/**
 * 新设备连接回调，当设备确认为k510后，用户程序在此处理
 */
PUBLIC on_device_handle_t kburnOnUsbConfirm(KBCTX scope, on_device_handle handle_callback, void *ctx);

/**
 * 开始监视串口+USB设备
 * 等于两个分别调用
 **/
PUBLIC kburn_err_t kburnStartWaitingDevices(KBCTX scope);
/**
 * 开始监视USB设备
 * **未实现**
 **/
PUBLIC kburn_err_t kburnStartWaitingUsbDevices(KBCTX scope);
/**
 * 开始监视串口设备
 * **未实现**
 **/
PUBLIC kburn_err_t kburnStartWaitingSerialDevices(KBCTX scope);

PUBLIC void kburnWaitDevicePause(KBCTX scope);
PUBLIC kburn_err_t kburnWaitDeviceResume(KBCTX scope);

/** 调试：获取需要删除的资源数 */
PUBLIC uint32_t kburnGetResourceCount();
/** 调试：获取打开的端口数 */
PUBLIC uint32_t kburnGetOpenPortCount();

/**
 * 直接打开端口，通常不应该调用这个函数
 * @param path USB设备路径
 */
PUBLIC kburn_err_t kburnOpenUsb(KBCTX scope, uint16_t vid, uint16_t pid, const uint8_t *path);

/**
 * 设备列表
 */
PUBLIC kburnSerialDeviceList kburnGetSerialList(KBCTX scope);
PUBLIC void kburnFreeSerialList(KBCTX scope);
PUBLIC kburnUsbDeviceList kburnGetUsbList(KBCTX scope);
PUBLIC void kburnFreeUsbList(KBCTX scope);

/**
 * 单次扫描
 */
PUBLIC kburn_err_t kburnPollSerial(KBCTX scope);
PUBLIC kburn_err_t kburnPollUsb(KBCTX scope);

PUBLIC void kburnDeviceRef(kburnDeviceNode *node);
PUBLIC void kburnDeviceUnRef(kburnDeviceNode *node);

DEFINE_END

#include "./exported/config-api.h"
#include "./exported/debug.h"
#include "./exported/serial.isp.h"
#include "./exported/usb.isp.h"

#ifdef KBURN_ASSERT_SIZE
#include "./exported/static-assert.h"
#endif
