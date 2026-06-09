/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file timer.c
 * @author Juuso Alasuutari
 */

#include "compat.h"

#ifdef _WIN32
# include <stdatomic.h>
# include <stdint.h>
#endif /* _WIN32 */

#include "timer.h"

#ifndef _WIN32
double
millis (timestamp a,
        timestamp b)
{
	return (double)((b.tv_sec -
	                 a.tv_sec) * 1000)
	       + (double)b.tv_nsec / 1000000
	       - (double)a.tv_nsec / 1000000;
}

#else /* _WIN32 */

static _Atomic(int64_t) timer_freq;

static inline int64_t
get_timer_freq (void)
{
	int64_t ret = atomic_load_explicit(&timer_freq,
	                                   memory_order_relaxed);
	if (!ret) {
		LARGE_INTEGER pf;
		QueryPerformanceFrequency(&pf);
		ret = (int64_t)pf.QuadPart;
		atomic_store_explicit(&timer_freq, ret,
		                      memory_order_release);
	}
	return ret;
}

double
millis (timestamp a,
        timestamp b)
{
	return (double)((b.QuadPart -
	                 a.QuadPart) * 1000)
	       / (double)get_timer_freq();
}
#endif /* _WIN32 */
