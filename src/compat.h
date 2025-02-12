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
# endif // clang_older_than_version(13)
# define _POSIX_C_SOURCE 199309L
# if clang_older_than_version(13)
diag_clang(pop)
# endif // clang_older_than_version(13)
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

#endif /* DBS26_SRC_COMPAT_H_ */
