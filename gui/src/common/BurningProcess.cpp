#include "BurningProcess.h"
#include "BurnLibrary.h"
#include "main.h"
#include "MyException.h"
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QFuture>
#include <QPromise>
#include <QThread>

BurningProcess::BurningProcess(KBCTX scope, const BurningRequest *request)
	: scope(scope), imageFile(request->systemImageFile), imageSize(imageFile.size()) {
	this->setAutoDelete(false);

	if (!imageFile.open(QIODeviceBase::ReadOnly)) {
		setResult(KBurnException(::tr("无法打开系统镜像文件") + " (" + request->systemImageFile + ")"));
		return;
	}
}

BurningProcess::~BurningProcess() {
	if (imageStream) {
		delete imageStream;
	}
	if (buffer) {
		delete buffer;
	}
}

void BurningProcess::setResult(const KBurnException &reason) {
	_result = reason;
}

void BurningProcess::schedule() {
	if (!_isStarted && !_isCanceled) {
		_isStarted = true;
		BurnLibrary::instance()->getThreadPool()->start(this);
	}
}

void BurningProcess::_run() {
	Q_ASSERT(_isStarted);

	QThread::currentThread()->setObjectName("burn:" + getTitle());
	kburn_stor_address_t address = 0; // TODO: config base

	throwIfCancel();

	imageStream = new QDataStream(&imageFile);

	qint64 chunkSize = prepare();

	buffer = new QByteArray(chunkSize, 0);
	setStage(::tr("下载中"), imageSize);

	while (!imageStream->atEnd()) {
		imageStream->readRawData(buffer->data(), buffer->size());

		int tries = 5; // TODO: config
		while (tries-- > 0) {
			if (step(address, *buffer)) {
				break;
			}

			// TODO: notify when tries>1

			if (tries == 1) {
				throw KBurnException(tr("设备写入失败，地址: 0x") + QString::number(address, 16));
			}
		}

		address += chunkSize;
		setProgress(address);
	}

	setStage(::tr("完成"), 100);
	emit completed();
}

void BurningProcess::run() Q_DECL_NOTHROW {
	try {
		_run();
		cleanup(true);
	} catch (KBurnException &e) {
		setResult(e); // may get result after return
		emit failed(_result);
		cleanup(false);
	} catch (...) {
		setResult(KBurnException("未知错误"));
		emit failed(_result);
		cleanup(false);
	}
	_isCompleted = true;
}

void BurningProcess::setProgress(int value) {
	throwIfCancel();
	emit progressChanged(value);
}

void BurningProcess::setStage(const QString &title, int bytes) {
	throwIfCancel();
	emit progressChanged(0);
	emit bytesChanged(bytes);
	emit stageChanged(title);
}

void BurningProcess::cancel(const KBurnException reason) {
	if (!_isCanceled) {
		_isCanceled = true;
		if (_result.errorCode == KBurnNoErr) {
			setResult(reason);
		}
		emit cancelRequested();
	}
}

void BurningProcess::cancel() {
	cancel(KBurnException(KBurnCommonError::KBurnUserCancel, ::tr("用户取消操作")));
}

void BurningProcess::throwIfCancel() {
	if (_result.errorCode != KBurnNoErr) {
		throw _result;
	}
	if (_isCanceled) {
		throw KBurnException(tr("用户主动取消"));
	}
}
