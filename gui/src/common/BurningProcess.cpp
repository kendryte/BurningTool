#include "BurningProcess.h"
#include <canaan-burn/canaan-burn.h>
#include <QFile>

static void handle_serial_progress(void *self, const kburnDeviceNode *dev, size_t current, size_t length) {
	reinterpret_cast<QPromise<void> *>(self)->setProgressValue(current);
}

static void run_progress(QPromise<void> &output, QFuture<kburnDeviceNode *> &input, KBCTX scope, const QString &comPort, const QString &systemImage) {
}

FlashTask::~FlashTask() {
	if (result) {
		delete result;
	}
}

void FlashTask::run() {
	try {
		QFile imageFile(systemImage);
		if (!imageFile.open(QIODeviceBase::ReadOnly)) {
			throw KBurnException(QString::fromUtf8("无法打开系统镜像文件") + " (" + systemImage + ")");
		}
		QTextStream imageStream(&imageFile);

		output.setProgressRange(0, 0);
		output.setProgressValueAndText(0, "waiting device...");

		const auto e = kburnOpenSerial(scope, comPort.toLatin1().data());
		if (e != KBurnNoErr) {
			throw KBurnException(e, "串口握手失败");
		}

		size_t totalSize = imageFile.size() + kburnGetUsbIspProgramSize();
		output.setProgressRange(0, totalSize);

		auto node = (kburnDeviceNode *)inputs.pick(0);

		if (!kburnSerialIspSetBaudrateHigh(node->serial)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		if (!kburnSerialIspSwitchUsbMode(node->serial, handle_serial_progress, &output)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		node = (kburnDeviceNode *)inputs.pick(1);

		kburnDeviceMemorySizeInfo devInfo;
		if (!kburnUsbIspGetMemorySize(node, kburnUsbIspCommandTaget::KBURN_USB_ISP_EMMC, &devInfo)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

	} catch (KBurnException e) {
		result = new KBurnException(e);
		output.setException(e);
		return;
	}
	output.finish();
}

void FlashTask::cancel() {
	// TODO: impl
}
