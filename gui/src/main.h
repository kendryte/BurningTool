#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QString>

QString tr(const char *s, const char *c = nullptr, int n = -1);
void saveWindowSize();

#ifdef NDEBUG
#define IS_DEBUG false
#else
#define IS_DEBUG true
#endif
