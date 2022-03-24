#include <canaan-burn/canaan-burn.h>
#include <stddef.h>
#include <iostream>

using namespace std;

const char *test_null(const char *str)
{
	return str ? str : "NULL";
}

bool verify(const kburnDeviceNode *dev, void *ctx)
{
	cout << "ask connect: " << dev->path << ", isUSB=" << dev->deviceInfo.isUSB << endl;
	return true;
}

void handle(kburnDeviceNode *dev, void *ctx)
{
	cout << "GotDev: " << dev->path << endl;
	cout << "  * isOpen: " << dev->isOpen << endl;
	cout << "  * isConfirm: " << dev->isConfirm << endl;
	cout << "  * error status: " << dev->error->code << ", " << test_null(dev->error->errorMessage) << endl;
}

void disconnect(const kburnDeviceNode *dev, void *ctx)
{
	cout << "Disconnect: " << dev->path << endl;
	cout << "  * isOpen: " << dev->isOpen << endl;
	cout << "  * isConfirm: " << dev->isConfirm << endl;
	cout << "  * error status: " << dev->error->code << ", " << test_null(dev->error->errorMessage) << endl;
}

int main()
{
	kburnRegisterDisconnectCallback(disconnect, NULL);
	kburnSetCallbackVerifyDevice(verify, NULL);
	kburnSetCallbackHandler(handle, NULL);

	kburnStartWaitingDevices();

	printf("Press ENTER to stop monitoring\n");
	getchar();

	kburnDispose();

	return 0;
}
