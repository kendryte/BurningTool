#include "BurningProcess.h"
#include <canaan-burn/canaan-burn.h>

void FlashTask::setSystemImageFile(const QString &systemImage) {
	this->systemImage = systemImage;
}
void FlashTask::run() {
	kburnOpenSerial(scope, systemImage.toLatin1().data());
}
