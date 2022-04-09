#include <windows.h>

size_t utf8_encode(const wchar_t *wstr, char *str, size_t len) {
	int wl = (int)wcslen(wstr);
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wl, NULL, 0, NULL, NULL);
	if (str == NULL) {
		return size_needed;
	}

	int s = WideCharToMultiByte(CP_UTF8, 0, wstr, wl, str, len, NULL, NULL);

	memset(str + s, 0, len - s);
	return s;
}
