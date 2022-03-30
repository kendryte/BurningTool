#include <canaan-burn/canaan-burn.h>
#include <stddef.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <iomanip>
#include <string>
#include <cstring>
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

static void perror(kburn_err_t e)
{
	auto errs = kburnSplitErrorCode(e);
	cout << "    - kind: " << (errs.kind >> 32) << endl;
	cout << "    - code: " << errs.code << endl;
}

void handle(kburnDeviceNode *dev, void *ctx)
{
	cout << "Got Serial Device: " << dev->serial->path << endl;
	cout << "  * isOpen: " << dev->serial->isOpen << endl;
	cout << "  * isConfirm: " << dev->serial->isConfirm << endl;
	cout << "  * error status: " << dev->error->code << ", " << test_null(dev->error->errorMessage) << endl;

	if (dev->error->code != KBurnNoErr)
	{
		perror(dev->error->code);
		return;
	}

	if (!kburnSerialIspSetBaudrateHigh(dev->serial))
	{
		cout << "Error: can not set baudrate: " << test_null(dev->error->errorMessage) << endl;
	}

	if (!kburnSerialIspSwitchUsbMode(dev->serial, print_progress, NULL))
	{
		cout << "Error: can not go usb mode: " << test_null(dev->error->errorMessage) << endl;
	}
}

void handle_usb(kburnDeviceNode *dev, void *ctx)
{
	kburn_err_t r;

	cout << "Got Usb Device: " << endl;
	cout << "  * serial port: " << (dev->serial->isUsbBound ? dev->serial->path : "not bind") << endl;
	cout << "  * usb port: ";
	for (auto i = 0; i < MAX_PATH_LENGTH; i++)
	{
		cout << (uint8_t)dev->usb->deviceInfo.path[i] << " " << flush;
	}
	cout << endl;
	cout << "    usb vid: 0x" << hex << dev->usb->deviceInfo.idVendor << dec << endl;
	cout << "    usb pid: 0x" << hex << dev->usb->deviceInfo.idProduct << dec << endl;
	cout << "    usb serial number: " << test_null((char *)dev->usb->deviceInfo.strSerial) << endl;

	cout << "  * error status: " << dev->error->code << ", " << test_null(dev->error->errorMessage) << endl;

	if (dev->error->code != KBurnNoErr)
		return;

	kburnDeviceMemorySizeInfo size_info;
	uint8_t test_block[512];
	for (size_t i = 0; i < 512; i++)
		test_block[i] = rand();
	uint32_t testAddress = 50 * 512;

	cout << "test read memory size:" << endl;
	r = kburnUsbIspGetMemorySize(dev, KBURN_USB_ISP_EMMC, &size_info);
	if (r != KBurnNoErr)
	{
		cout << "  * error status: " << dev->error->code << ", " << test_null(dev->error->errorMessage) << endl;
	}
	else
	{
		cout << "  * block_size: " << size_info.block_size << endl;
		cout << "  * device_size: " << size_info.device_size << endl;
		cout << "  * max_block_addr: " << size_info.max_block_addr << endl;
	}

	cout << "test write emmc:" << endl;
	r = kburnUsbIspWriteData(dev, KBURN_USB_ISP_EMMC, testAddress, test_block, 512, &size_info);
	perror(r);

	cout << "test read emmc:" << endl;
	uint8_t out_test[512];
	memset(out_test, 0, 512);
	r = kburnUsbIspReadData(dev, KBURN_USB_ISP_EMMC, testAddress, 512, out_test, &size_info);
	perror(r);
	if (memcmp(out_test, test_block, 0) == 0)
		cout << "    - data same" << endl;
	else
		cout << "    - data wrong" << endl;
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
	assert(KBURN_ERROR_KIND_SERIAL > UINT32_MAX);
	cout << '\x1B' << 'c';
	flush(cout);

	KBCTX context = NULL;
	kburn_err_t err = kburnCreate(&context);
	if (err != KBurnNoErr)
	{
		cerr << "Failed Start: " << err << endl;
		return 1;
	}

	kburnOnDeviceDisconnect(context, disconnect, NULL);
	kburnOnSerialConnect(context, verify, NULL);
	kburnOnSerialConfirm(context, handle, NULL);

	kburnOnUsbConfirm(context, handle_usb, NULL);

	kburnStartWaitingDevices(context);

	// kburnOpenSerial(context, "/dev/ttyUSB0");
	// testDevice(argv[1]);

	// kburnOpen("/dev/ttyUSB3");
	// kburnOpen("/dev/ttyUSB4");
	// kburnOpen("/dev/ttyUSB5");

	// kburnOpenUSB(context, 0x0559, 0x4001, (const uint8_t *)"4b7e4b47");

	printf("Press ENTER to stop monitoring\n");
	getchar();

	kburnGlobalDestroy();

	return 0;
}
