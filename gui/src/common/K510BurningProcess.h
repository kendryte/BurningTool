#include "BurningProcess.h"
#include "EventStack.h"
#include <QString>

class K510BurningProcess : public BurningProcess {
  private:
	EventStack inputs;
	kburnDeviceMemorySizeInfo devInfo;
	kburnDeviceNode *node = NULL;
	const QString comPort;
	const QString _identity;

	static void serial_isp_progress(void *, const kburnDeviceNode *, size_t, size_t);

	qint64 prepare();
	bool step(kburn_stor_address_t address, const QByteArray &chunk);

  public:
	K510BurningProcess(KBCTX scope, const K510BuringRequest *request);
	QString getTitle() const;

	bool pollingDevice(kburnDeviceNode *node, BurnLibrary::DeviceEvent event);
	const QString &getIdentity() const { return _identity; }
};