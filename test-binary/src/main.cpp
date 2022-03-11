#include <canaan-burn/canaan-burn.h>
#include <stddef.h>
#include <iostream>
#include "main.h"

using namespace std;

const char *test_null(const char *str)
{
	return str ? str : "NULL";
}

bool verify(const kburnDeviceNode *dev, void *ctx)
{
	cout << "ask connect: " << dev->serial->path << ", isUSB=" << dev->serial->deviceInfo.isUSB << endl;
	return true;
}

void print_progress(const struct kburnDeviceNode *dev, size_t current, size_t length, void *ctx)
{
	cout << "loading: " << current << '/' << length << "\t " << (int)(current * 100 / length) << "%" << endl;
}

void handle(kburnDeviceNode *dev, void *ctx)
{
	cout << "GotDev: " << dev->serial->path << endl;
	cout << "  * isOpen: " << dev->serial->isOpen << endl;
	cout << "  * isConfirm: " << dev->serial->isConfirm << endl;
	cout << "  * error status: " << dev->error->code << ", " << test_null(dev->error->errorMessage) << endl;

	if (!kburnSerialIspSetBaudrateHigh(dev->serial))
	{
		cout << "Error: can not set baudrate: " << test_null(dev->error->errorMessage) << endl;
	}

	if (!kburnSerialIspSwitchUsbMode(dev->serial, print_progress, NULL))
	{
		cout << "Error: can not go usb mode: " << test_null(dev->error->errorMessage) << endl;
	}
}

void disconnect(const kburnDeviceNode *dev, void *ctx)
{
	cout << "Disconnect: " << dev->serial->path << endl;
	cout << "  * isOpen: " << dev->serial->isOpen << endl;
	cout << "  * isConfirm: " << dev->serial->isConfirm << endl;
	cout << "  * error status: " << dev->error->code << ", " << test_null(dev->error->errorMessage) << endl;
}

int main(int argc, char **argv)
{
	KBCTX context = NULL;
	kburn_err_t err = kburnCreate(&context);
	if (err != KBurnNoErr)
	{
		cerr << "Failed Start: " << err << endl;
		return 1;
	}

	kburnOnSerialDisconnect(context, disconnect, NULL);
	kburnOnSerialConnect(context, verify, NULL);
	kburnOnSerialConfirm(context, handle, NULL);

	// kburnWaitDeviceInitStart();

	kburnOpenSerial(context, "/dev/ttyUSB0");
	// testDevice(argv[1]);

	// kburnOpen("/dev/ttyUSB3");
	// kburnOpen("/dev/ttyUSB4");
	// kburnOpen("/dev/ttyUSB5");

	printf("Press ENTER to stop monitoring\n");
	getchar();

	kburnGlobalDestroy();

	return 0;
}
