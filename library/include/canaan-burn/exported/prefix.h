#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
#define DEFINE_START extern "C" {
#define DEFINE_END }
#else
#define DEFINE_START
#define DEFINE_END
#endif

#if __clang__
#define arg_readonly(...)
#define arg_writeonly(...)
#define arg_cb(fn, arg) __attribute__((callback(fn, arg)))
#elif __GNUC__
#define arg_readonly(A, B) __attribute__((access(read_only, A, B)))
#define arg_writeonly(A, B) __attribute__((access(write_only, A, B)))
#define arg_cb(...)
#else
#define arg_readonly(...)
#define arg_writeonly(...)
#define arg_cb(...)
#endif
