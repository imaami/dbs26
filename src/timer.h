/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file timer.h
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_TIMER_H_
#define DBS26_SRC_TIMER_H_

#include "compat.h"

#ifndef _WIN32
# include <time.h>
#else /* _WIN32 */
# include <Windows.h>
#endif /* _WIN32 */

#ifndef _WIN32
static force_inline struct timespec
now (void)
{
	struct timespec t = {0};
	(void)clock_gettime(CLOCK_MONOTONIC, &t);
	return t;
}
#else /* _WIN32 */
static force_inline LARGE_INTEGER
now (void)
{
	LARGE_INTEGER t = {0};
	(void)QueryPerformanceCounter(&t);
	return t;
}
#endif /* _WIN32 */

typedef typeof (now()) timestamp;

extern double
millis (timestamp a,
        timestamp b);

static force_inline double
since (timestamp const t)
{
	return millis(t, now());
}

#endif /* DBS26_SRC_TIMER_H_ */
