#pragma once

#include <canaan-burn/canaan-burn.h>
#include <QString>

void fatalAlert(kburn_err_t err);
QString tr(const char *s, const char *c = nullptr, int n = -1);
