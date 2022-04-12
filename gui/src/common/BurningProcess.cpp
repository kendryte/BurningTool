#include "BurningProcess.h"
#include "main.h"
#include <canaan-burn/canaan-burn.h>
#include <QFile>
#include <QFileInfo>
#include <QFuture>
#include <QThread>

#define CHUNK_SIZE 1024 * 1024

static void run_progress(QPromise<void> &output, QFuture<kburnDeviceNode *> &input, KBCTX scope, const QString &comPort, const QString &systemImage) {
}

FlashTask::~FlashTask() {
	if (result) {
		delete result;
	}
}

void FlashTask::setProgressValue(size_t value) {
	output.setProgressValue(bytesWritten + value);
	qWarning() << "set progress: " << value;
}

void FlashTask::nextStage(const QString &title, size_t bytes) {
	bytesWritten += bytesNextStage;
	bytesNextStage = bytes;
	output.setProgressRange(bytesWritten, bytesWritten + bytes);
	emit progressTextChanged(title);
	qWarning() << "set stage: " << title << " bytes: " << bytes;
}

void FlashTask::serial_isp_progress(void *self, const kburnDeviceNode *dev, size_t current, size_t length) {
	auto _this = reinterpret_cast<FlashTask *>(self);
	_this->setProgressValue(current);
}

void FlashTask::run() {
	try {
#if __linux__
		QThread::currentThread()->setObjectName("burn:" + QFileInfo(comPort).baseName());
#elif WIN32
		QThread::currentThread()->setObjectName("burn:" + comPort + ":" + QFileInfo(systemImage).fileName());
#else
		TODO;
#endif

		QFile imageFile(systemImage);
		if (!imageFile.open(QIODeviceBase::ReadOnly)) {
			throw KBurnException(::tr("无法打开系统镜像文件") + " (" + systemImage + ")");
		}
		QDataStream imageStream(&imageFile);

		nextStage(::tr("正在打开串口设备"));

		const auto e = kburnOpenSerial(scope, comPort.toLatin1().data());
		if (e != KBurnNoErr) {
			throw KBurnException(e, ::tr("串口握手失败"));
		}

		nextStage(::tr("写入USB ISP"), kburnGetUsbIspProgramSize());

		auto node = (kburnDeviceNode *)inputs.pick(0);
		if (node == NULL) {
			throw KBurnException(tr("设备超时"));
		}

		if (!kburnSerialIspSetBaudrateHigh(node->serial)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		if (!kburnSerialIspSwitchUsbMode(node->serial, FlashTask::serial_isp_progress, this)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		nextStage(::tr("等待USB ISP启动"));

		node = (kburnDeviceNode *)inputs.pick(1);
		if (node == NULL) {
			throw KBurnException(tr("设备超时"));
		}

		kburnDeviceMemorySizeInfo devInfo;
		if (!kburnUsbIspGetMemorySize(node, kburnUsbIspCommandTaget::KBURN_USB_ISP_EMMC, &devInfo)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		auto imageSize = imageFile.size();
		if (imageSize > devInfo.storage_size) {
			throw KBurnException(tr("文件过大"));
		}
		nextStage(::tr("下载中"), imageSize);

		size_t chunk_size = ((CHUNK_SIZE / devInfo.block_size) + (CHUNK_SIZE % devInfo.block_size > 0 ? 1 : 0)) * devInfo.block_size;
		char readBuffer[chunk_size];
		memset(readBuffer, 0, chunk_size);
		size_t block = 0, readProgress = 0;
		while (!imageStream.atEnd()) {
			size_t actread = imageStream.readRawData(readBuffer, chunk_size);

			bool success = [&] {
				for (int tries = 0; tries < 5; tries++) {
					if (kburnUsbIspWriteChunk(node, devInfo, block, readBuffer, chunk_size)) {
						return true;
					} else {
						fprintf(stderr, "\x1b[38;5;9mkburnUsbIspWriteChunk failed [%d/5]: %s\x1b[0m\n", tries + 1, node->error->errorMessage);
					}
				}
				return false;
			}();

			if (!success) {
				throw KBurnException(tr("设备写入失败，地址: ") + QString::number(block * chunk_size, 16));
			}
			testCancel();

			block += chunk_size / devInfo.block_size;

			readProgress += actread;
			setProgressValue(readProgress);
		}
	} catch (KBurnException e) {
		result = new KBurnException(e); // copy: prevent access free-ed error message, free in ~FlashTask
		output.setException(e);
		return;
	}

	nextStage(::tr(""), 100);
	output.finish();
}

void FlashTask::cancel() {
	canceled = true;
}

void FlashTask::testCancel() {
	if (canceled) {
		throw KBurnException(tr("用户主动取消"));
	}
}

QFuture<void> FlashTask::future() const {
	return output.future();
}

void FlashTask::onSerialConnected(kburnDeviceNode *node) {
	inputs.set(0, node);
	emit onDeviceChange(node);
};
void FlashTask::onUsbConnected(kburnDeviceNode *node) {
	inputs.set(1, node);
	emit onDeviceChange(node);
};
