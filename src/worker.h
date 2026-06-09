/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file worker.h
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_WORKER_H_
#define DBS26_SRC_WORKER_H_

#include "compat.h"

#ifndef _WIN32
# include <pthread.h>
#endif

#include <stddef.h>
#include <stdint.h>

typedef uintptr_t worker_cb (void *);

#ifndef _WIN32
typedef pthread_t worker_tid;
#else
typedef uintptr_t worker_tid;
#endif

struct worker {
	worker_tid  tid;
	worker_cb  *cb;
	union {
		void      *ctx;
		uintptr_t  ret;
	};
};

extern int
worker_start (struct worker *w,
              worker_cb     *cb,
              void          *ctx);

extern int
worker_wait (struct worker *w);

#endif /* DBS26_SRC_WORKER_H_ */
