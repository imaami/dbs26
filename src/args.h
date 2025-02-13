/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file args.h
 * @brief Command-line argument parsing
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_ARGS_H_
#define DBS26_SRC_ARGS_H_

#include <stdint.h>

struct args {
	uintptr_t   have;
	char const *output;
	uint32_t    threads;
	int32_t     error;
};

extern struct args
args (int    argc,
      char **argv);

#endif /* DBS26_SRC_ARGS_H_ */
