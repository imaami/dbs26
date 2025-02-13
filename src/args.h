/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file args.h
 * @brief Command-line argument parsing
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_ARGS_H_
#define DBS26_SRC_ARGS_H_

#include <stdint.h>
#include <stdio.h>

struct args {
	uintptr_t   have;
	char const *output;
	uint32_t    threads;
	int32_t     error;
};

typedef void args_usage_fn (FILE       *stream,
                            char const *argv0);

extern struct args
args (int            argc,
      char          *argv[],
      args_usage_fn *usage);

#endif /* DBS26_SRC_ARGS_H_ */
