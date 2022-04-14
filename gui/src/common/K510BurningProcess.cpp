#include "K510BurningProcess.h"
#include "main.h"
#include "MyException.h"
#include <canaan-burn/canaan-burn.h>
#include <QFileInfo>

#define CHUNK_SIZE 1024 * 1024

K510BurningProcess::K510BurningProcess(KBCTX scope, const K510BuringRequest *request)
	: BurningProcess(scope, request), comPort(request->comPort), inputs(2), _identity(typeid(*this).name() + comPort) {
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
	setStage(::tr("正在打开串口设备"));

	const auto e = kburnOpenSerial(scope, (const char *)comPort.constData());
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

	if (!kburnUsbIspGetMemorySize(node, kburnUsbIspCommandTaget::KBURN_USB_ISP_EMMC, &devInfo)) {
		throw KBurnException(node->error->code, node->error->errorMessage);
	}

	if (imageSize > devInfo.storage_size) {
		throw KBurnException(tr("文件过大"));
	}

	return ((CHUNK_SIZE / devInfo.block_size) + (CHUNK_SIZE % devInfo.block_size > 0 ? 1 : 0)) * devInfo.block_size;
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
	emit deviceStateNotify(node);
	return true;
}
