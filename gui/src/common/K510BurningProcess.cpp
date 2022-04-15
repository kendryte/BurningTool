#include "K510BurningProcess.h"
#include "main.h"
#include "MyException.h"
#include <canaan-burn/canaan-burn.h>
#include <QFileInfo>

#define CHUNK_SIZE 1024 * 1024

K510BurningProcess::K510BurningProcess(KBCTX scope, const K510BurningRequest *request)
	: BurningProcess(scope, request), comPort(request->comPort), inputs(2) {
	this->setAutoDelete(false);
};

QString K510BurningProcess::getTitle() const {
#if WIN32
	return comPort;
#else
	return QFileInfo(comPort).baseName();
#endif
}

void K510BurningProcess::serial_isp_progress(void *self, const kburnDeviceNode *dev, size_t current, size_t length) {
	auto _this = reinterpret_cast<K510BurningProcess *>(self);
	_this->setProgress(current);
}

qint64 K510BurningProcess::prepare() {
	// FIXME 串口打开过程中（没开始写isp）断开链接会崩溃

	setStage(::tr("正在打开串口设备"));

	const auto e = kburnOpenSerial(scope, (const char *)comPort.toLatin1());
	if (e != KBurnNoErr) {
		throw KBurnException(e, ::tr("串口握手失败"));
	}

	setStage(::tr("写入USB ISP"), kburnGetUsbIspProgramSize());

	node = reinterpret_cast<kburnDeviceNode *>(inputs.pick(0));
	if (node == NULL) {
		throw KBurnException(tr("设备超时"));
	}

	if (!kburnSerialIspSetBaudrateHigh(node->serial)) {
		throw KBurnException(node->error->code, node->error->errorMessage);
	}

	if (!kburnSerialIspSwitchUsbMode(node->serial, K510BurningProcess::serial_isp_progress, this)) {
		throw KBurnException(node->error->code, node->error->errorMessage);
	}

	setStage(::tr("等待USB ISP启动"));

	node = reinterpret_cast<kburnDeviceNode *>(inputs.pick(1));
	if (node == NULL) {
		throw KBurnException(tr("设备超时"));
	}

	usb_ok = true;

	if (!kburnUsbIspGetMemorySize(node, kburnUsbIspCommandTaget::KBURN_USB_ISP_EMMC, &devInfo)) {
		throw KBurnException(node->error->code, node->error->errorMessage);
	}

	if (imageSize > devInfo.storage_size) {
		throw KBurnException(tr("文件过大"));
	}

	return ((CHUNK_SIZE / devInfo.block_size) + (CHUNK_SIZE % devInfo.block_size > 0 ? 1 : 0)) * devInfo.block_size;
}

#include "AppGlobalSetting.h"
void K510BurningProcess::cleanup(bool success) {
	if (!usb_ok) {
		return;
	}
	uint32_t color = GlobalSetting::usbLedLevel.getValue();
	if (success) {
		color = color << 8;
	} else {
		color = color << 16;
	}
	kburnUsbIspLedControl(node, GlobalSetting::usbLedPin.getValue(), kburnConvertColor(color));
}

bool K510BurningProcess::step(kburn_stor_address_t address, const QByteArray &chunk) {
	size_t block = address / devInfo.block_size;

	return kburnUsbIspWriteChunk(node, devInfo, block, (void *)chunk.constData(), chunk.size());
}

bool K510BurningProcess::pollingDevice(kburnDeviceNode *node, BurnLibrary::DeviceEvent event) {
	bool intresting = this->comPort == node->serial->deviceInfo.path;
	if (!intresting) {
		return false;
	}
	if (event == BurnLibrary::DeviceEvent::SerialReady) {
		inputs.set(0, node);
	} else if (event == BurnLibrary::DeviceEvent::UsbReady) {
		inputs.set(1, node);
	} else if (event == BurnLibrary::DeviceEvent::Disconnected) {
		this->cancel(KBurnException(::tr("设备断开")));
	}
	emit deviceStateNotify();
	return true;
}

void K510BurningProcess::recreateDeviceStatus(const kburnDeviceNode *dev) {
	QString &val = _detailInfo;
	val += tr("Serial Device: ");
	if (dev->serial->init || dev->serial->isUsbBound) {
		val += dev->serial->deviceInfo.path;
		val += '\n';
		val += tr("  * init: ");
		val += dev->serial->init ? tr("yes") : tr("no");
		val += '\n';
		val += tr("  * isOpen: ");
		val += dev->serial->isOpen ? tr("yes") : tr("no");
		val += '\n';
		val += tr("  * isConfirm: ");
		val += dev->serial->isConfirm ? tr("yes") : tr("no");
		val += '\n';
		val += tr("  * isUsbBound: ");
		val += dev->serial->isUsbBound ? tr("yes") : tr("no");
	} else {
		val += tr("not connected");
	}
	val += '\n';

	val += tr("Usb Device: ");
	if (dev->usb->init) {
		for (auto i = 0; i < MAX_USB_PATH_LENGTH - 1; i++) {
			val += QString::number(dev->usb->deviceInfo.path[i], 16) + QChar(':');
		}
		val.chop(1);
		val += '\n';

		val += "  * VID: " + QString::number(dev->usb->deviceInfo.idVendor, 16) + '\n';
		val += "  * PID: " + QString::number(dev->usb->deviceInfo.idProduct, 16) + '\n';
	} else {
		val += tr("not connected");
	}
	val += '\n';

	if (dev->error->code) {
		val += tr("  * error status: ");
		auto errs = kburnSplitErrorCode(dev->error->code);
		val += tr("kind: ");
		val += QString::number(errs.kind >> 32);
		val += ", ";
		val += tr("code: ");
		val += QString::number(errs.code);
		val += ", ";
		val += QString::fromLatin1(dev->error->errorMessage);
	}
}
