/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file dbs26.c
 * @brief Generate all 67108864 unique binary De Bruijn sequences
 *        with subsequence length 6, ordered by value.
 * @author Juuso Alasuutari
 */

#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "solver.h"

int
main (int    argc,
      char **argv)
{
	struct args a = args(argc, argv);

	int e = 0;
	struct solver *s = solver_create(a.threads, &e);
	if (!s) {
		(void)fprintf(stderr, "solver_create: %s\n", strerror(e));
		return EXIT_FAILURE;
	}

	solver_solve(s, a.output);
	solver_destroy(&s);

	return EXIT_SUCCESS;
}
