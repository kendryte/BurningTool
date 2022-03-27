#if WIN32
#include <windows.h>
void do_sleep(int ms)
{
	Sleep(ms);
}
#else
#include <unistd.h>
void do_sleep(int ms)
{
	usleep(ms * 1000);
}
#endif
