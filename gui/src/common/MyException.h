#pragma once

#include "main.h"
#include <stdexcept>
#include <canaan-burn/canaan-burn.h>
#include <QException>

class KBurnException : public QException {
  public:
	KBurnException() : QException(), errorCode(KBurnNoErr), errorMessage(::tr("没有发生错误")) {}

	KBurnException(const KBurnException &other) : QException(), errorCode(other.errorCode), errorMessage(other.errorMessage + "") {}

	KBurnException(kburn_err_t errorCode, const QString &message) : QException(), errorCode(errorCode), errorMessage(message) {
		Q_ASSERT(errorCode != KBurnNoErr);
	}

	KBurnException(const QString &errorMessage) : QException(), errorCode((uint64_t)UINT32_MAX << 32 | 1), errorMessage(errorMessage) {
		Q_ASSERT(errorCode != KBurnNoErr);
	}

	KBurnException &operator=(const KBurnException &other) {
		errorCode = other.errorCode;
		errorMessage = other.errorMessage;
		return *this;
	}

	kburn_err_t errorCode;
	QString errorMessage;

	void raise() const override { throw *this; }
};
