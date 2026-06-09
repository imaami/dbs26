/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file task.h
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_TASK_H_
#define DBS26_SRC_TASK_H_

#include "buf64.h"

#define SUB_LEN 6U
#define SEARCH_SPACE(n) ((1U << n) - n - 2U)
#define SEARCH_DEPTH(n) (SEARCH_SPACE(n) / n)

struct task {
	uint32_t sp;
	uint32_t sum;
	struct {
		uint64_t end;
		uint64_t map;
	} stk[SEARCH_DEPTH(SUB_LEN) - 1U];
};

extern struct buf64
task_solve (struct task *tsk,
            uint64_t    *dst,
            uint64_t     map,
            uint32_t     cnt,
            uint16_t     pfx);

#endif /* DBS26_SRC_TASK_H_ */
