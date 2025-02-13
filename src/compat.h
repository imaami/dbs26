/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file compat.h
 * @brief Cross-platform compatibility header
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_COMPAT_H_
#define DBS26_SRC_COMPAT_H_

#ifdef __cplusplus
# error "This is C and won't compile in C++ mode."
#endif

#include "compiler.h"
#include "pragma.h"

#ifdef _WIN32
# define _CRT_SECURE_NO_WARNINGS
# define WIN32_LEAN_AND_MEAN
#endif

// For clock_gettime() and CLOCK_MONOTONIC
#if !defined _WIN32 && \
     defined __STRICT_ANSI__ && !defined _POSIX_C_SOURCE
# if clang_older_than_version(13)
diag_clang(push)
diag_clang(ignored "-Wreserved-id-macro")
# endif // __clang_major__ < 13
# define _POSIX_C_SOURCE 199309L
# if clang_older_than_version(13)
diag_clang(pop)
# endif // __clang_major__ < 13
#endif // !_WIN32 && __STRICT_ANSI__ && !_POSIX_C_SOURCE

#undef HAVE_C23_BOOL
#undef HAVE_C23_NULLPTR

#if __STDC_VERSION__ >= 202000L && !defined __INTELLISENSE__
# if gcc_at_least_version(13,1) || clang_at_least_version(15)
#  define HAVE_C23_BOOL
# endif
# if gcc_at_least_version(13,1) || clang_at_least_version(16)
#   define HAVE_C23_NULLPTR
# endif
#endif // __STDC_VERSION__ >= 202000L && !__INTELLISENSE__

#ifndef HAVE_C23_BOOL
# include <stdbool.h>
#endif

#ifndef HAVE_C23_NULLPTR
# include <stddef.h>
# define nullptr NULL
#endif

#undef HAVE_C23_BOOL
#undef HAVE_C23_NULLPTR

// Old Clang versions don't know new Doxygen commands
#if clang_older_than_version(10)
diag_clang(ignored "-Wdocumentation-unknown-command")
#endif // __clang_major__ < 10

// Complains about C99 syntax
#if clang_at_least_version(14)
diag_clang(ignored "-Wdeclaration-after-statement")
#endif // __clang_major__ >= 14

// Mostly false positives in C
#if clang_at_least_version(16)
diag_clang(ignored "-Wunsafe-buffer-usage")
#endif // __clang_major__ >= 16

// These whine about C23 when compiling C23
#if __STDC_VERSION__ >= 202000L
# if clang_at_least_version(16) && clang_older_than_version(18)
diag_clang(ignored "-Wpre-c2x-compat")
# endif // 16 <= __clang_major__ < 18
# if clang_at_least_version(18)
diag_clang(ignored "-Wpre-c23-compat")
# endif // __clang_major__ >= 18
#endif // __STDC_VERSION__ >= 202000L

// Complains about C11 when compiling C11
#if clang_at_least_version(19)
diag_clang(ignored "-Wpre-c11-compat")
#endif // __clang_major__ >= 19

#ifndef _MSC_VER
# define force_inline __attribute__((always_inline)) inline
# define const_inline __attribute__((const)) force_inline
#else // _MSC_VER
# define force_inline __forceinline
# define const_inline __forceinline
#endif // _MSC_VER

#endif /* DBS26_SRC_COMPAT_H_ */
