/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file nproc.c
 * @author Juuso Alasuutari
 */

#include "compat.h"

#include <limits.h>

#ifndef _WIN32
# include <unistd.h>
#else
# include <Windows.h>
#endif

#include "nproc.h"

uint32_t
nproc (void)
{
#ifndef _WIN32
# ifdef _SC_NPROCESSORS_ONLN
	long n = sysconf(_SC_NPROCESSORS_ONLN);
	if (n > 0L) {
#  if LONG_MAX > UINT32_MAX
		if (n > (long)UINT32_MAX)
			n = (long)UINT32_MAX;
#  endif
		return (uint32_t)n;
	}
# endif // _SC_NPROCESSORS_ONLN
#else // _WIN32
	DWORD n = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	if (n > 0)
		return (uint32_t)n;
#endif // _WIN32
	return 1U;
}
