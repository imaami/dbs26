/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file worker.c
 * @author Juuso Alasuutari
 */
#include "compat.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
# include <Windows.h>
# include <process.h>
# include <processthreadsapi.h>
# include <synchapi.h>
#endif

#include "worker.h"

#ifndef _WIN32
static void *
#else
static unsigned __stdcall
#endif
worker_thread (void *arg)
{
	struct worker *w = arg;
	arg = w->ctx;
	w->ret = w->cb(arg);

#ifndef _WIN32
	return NULL;
#else
	_endthreadex(0);
# ifdef _MSC_VER
	return 0;
# endif // _MSC_VER
#endif // _WIN32
}

int
worker_start (struct worker *w,
              worker_cb     *cb,
              void          *ctx)
{
	*w = (struct worker){
		.cb  = cb,
		.ctx = ctx
	};

#ifndef _WIN32
	return pthread_create(&w->tid, nullptr,
	                      worker_thread, w);
#else
	w->tid = _beginthreadex(nullptr, 0,
	                        worker_thread,
	                        w, 0, nullptr);
	return w->tid ? 0 : errno;
#endif
}

int
worker_wait (struct worker *w)
{
	int e = 0;

#ifndef _WIN32
	void *r = nullptr;
	e = pthread_join(w->tid, &r);
# ifndef __ANDROID__
	if (!e && r == PTHREAD_CANCELED)
		e = ECANCELED;
# endif // __ANDROID__
	if (e)
		(void)fprintf(stderr, "pthread_join: %s\n",
		              strerror(e));
#else // _WIN32
	DWORD r = WaitForSingleObject((HANDLE)w->tid, INFINITE);
	if (r == WAIT_OBJECT_0) {
		r = 0;
		if (GetExitCodeThread((HANDLE)w->tid, &r))
			e = (int)r;
	}
	CloseHandle((HANDLE)w->tid);
#endif // _WIN32

	w->tid = 0;
	return e;
}
