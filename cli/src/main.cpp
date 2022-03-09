#include <canaan-burn/canaan-burn.h>
#include <stddef.h>
#include <iostream>

using namespace std;

bool verify(const struct kburnSerialNode *dev)
{
	cout << "ask connect: " << dev->path << endl;
	return true;
}

void handle(const struct kburnSerialNode *dev)
{
	cout << "GotDev: " << dev->path << endl;
	cout << "isOpen: " << dev->isOpen << endl;
	cout << "isConfirm: " << dev->isConfirm << endl;
	cout << "error status: " << dev->isError << ", " << dev->errorMessage << endl;
}

int main()
{
	kburnWaitDevice(verify, handle);

	printf("Press ENTER to stop monitoring\n");
	getchar();

	kburnDispose();

	return 0;
}
