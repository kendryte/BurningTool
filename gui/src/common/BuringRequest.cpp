#include "BuringRequest.h"
#include "K510BurningProcess.h"

class BurningProcess *BuringRequest::reqeustFactory(KBCTX scope, const BuringRequest *request) {
	switch (request->getKind()) {
	case K510:
		return new K510BurningProcess(scope, reinterpret_cast<const K510BuringRequest *>(request));
	default:
		qFatal("invalid device kind argument.");
	}
}
