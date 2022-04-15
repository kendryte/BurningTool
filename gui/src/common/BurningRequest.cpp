#include "BurningRequest.h"
#include "K510BurningProcess.h"

class BurningProcess *BurningRequest::reqeustFactory(KBCTX scope, const BurningRequest *request) {
	switch (request->getKind()) {
	case K510:
		return new K510BurningProcess(scope, reinterpret_cast<const K510BurningRequest *>(request));
	default:
		qFatal("invalid device kind argument.");
	}
}
