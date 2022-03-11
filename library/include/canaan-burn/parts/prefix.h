#pragma once

#if BURN_LIB_COMPILING
#define PUBLIC EXPORT
#define PCONST
#else
#define PUBLIC IMPORT
#define PCONST const
#endif

#if defined(_MSC_VER)
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else
#pragma error Unknown dynamic link import / export semantics.
#endif

#ifdef __cplusplus
#define DEFINE_START \
	extern "C"       \
	{
#define DEFINE_END }
#else
#define DEFINE_START
#define DEFINE_END
#include <assert.h>
#endif
