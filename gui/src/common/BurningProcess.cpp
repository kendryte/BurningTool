#include "BurningProcess.h"
#include "main.h"
#include <canaan-burn/canaan-burn.h>
#include <QFile>
#include <QFileInfo>

#define CHUNK_SIZE 1024 * 1024

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

void FlashTask::nextStage(const QString &title, size_t bytes) {
	output.setProgressRange(0, bytes);
	output.setProgressValue(0);
	emit progressTextChanged(title);
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

		if (!kburnSerialIspSetBaudrateHigh(node->serial)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		if (!kburnSerialIspSwitchUsbMode(node->serial, handle_serial_progress, &output)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		nextStage(::tr("等待USB ISP启动"));

		node = (kburnDeviceNode *)inputs.pick(1);

		kburnDeviceMemorySizeInfo devInfo;
		if (!kburnUsbIspGetMemorySize(node, kburnUsbIspCommandTaget::KBURN_USB_ISP_EMMC, &devInfo)) {
			throw KBurnException(node->error->code, node->error->errorMessage);
		}

		auto imageSize = imageFile.size();
		if (imageSize > devInfo.storage_size) {
			throw KBurnException(tr("文件过大"));
		}
		nextStage(::tr("下载中"), imageSize);

		size_t chunk_size = (CHUNK_SIZE / devInfo.block_size + (CHUNK_SIZE % devInfo.block_size > 0 ? 1 : 0)) * devInfo.block_size;
		// imageStream.rea(chunk_size)
		char readBuffer[chunk_size];
		size_t block = 0;
		while (!imageStream.atEnd()) {
			imageStream.readRawData(readBuffer, chunk_size);

			bool success = [&] {
				for (int tries = 0; tries < 5; tries++) {
					if (kburnUsbIspWriteChunk(node, devInfo, block, readBuffer, chunk_size)) {
						return true;
					} else {
						qErrnoWarning("kburnUsbIspWriteChunk failed [%d/5]: %s", tries + 1, node->error->errorMessage);
					}
				}
				return false;
			}();

			if (!success) {
				throw KBurnException(tr("设备写入失败，地址: ") + QString::number(block * chunk_size, 16));
			}

			block += chunk_size / devInfo.block_size;
			qDebug() << "\x1B[38;5;9m" << block << "\x1B[0m" << QChar::LineFeed;
			output.setProgressValue(block * chunk_size);
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
