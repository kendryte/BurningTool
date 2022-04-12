#include "config.h"
#include "main.h"
#include <QString>

QString getTitleVersion() {
	QString r;
	r += " (" + ::tr("version") + ": " + QString::fromLatin1(VERSION_STRING) + " ";

#ifndef NDEBUG
	r += ::tr("Debug");
#else
	r += ::tr("Release");
#endif
#if !IS_CI
	r += " * " + ::tr("local");
#endif
	r += ")";
	return r;
}
