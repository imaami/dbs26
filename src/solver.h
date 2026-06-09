/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file solver.h
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_SOLVER_H_
#define DBS26_SRC_SOLVER_H_

#include <stdint.h>

struct solver;

extern struct solver *
solver_create (uint32_t  n_workers,
               int      *err);

extern void
solver_destroy (struct solver **pp);

extern void
solver_solve (struct solver *s,
              char const    *out);

#endif /* DBS26_SRC_SOLVER_H_ */
