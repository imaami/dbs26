/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file args.c
 * @brief Command-line argument parsing
 * @author Juuso Alasuutari
 */

#include "compat.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"

enum opt {
	OPT_NONE      = 0U,
	OPT_OUTPUT    = 1U << 0U,
	OPT_THREADS   = 1U << 1U,
	OPT_BENCHMARK = 1U << 2U,
	OPT_HELP      = 1U << 3U,
};

#define OPT_OK(X)                                            \
 ( X(OPT_NONE                                              ) \
 | X(         OPT_OUTPUT                                   ) \
 | X(                    OPT_THREADS                       ) \
 | X(         OPT_OUTPUT|OPT_THREADS                       ) \
 | X(                                OPT_BENCHMARK         ) \
 | X(                    OPT_THREADS|OPT_BENCHMARK         ) \
 | X(                                              OPT_HELP) )

static int
parse_u32 (uint32_t *dst,
           char     *src,
           uint32_t  min);

static force_inline bool
args_conflict (struct args const *a);

static int
args_help (struct args const *a,
           char const        *v0,
           args_usage_fn     *fn);

struct args
args (int const            argc,
      char **const         argv,
      args_usage_fn *const usage)
{
	struct args r = {
		.have = OPT_NONE,
		.output = nullptr,
		.threads = 0U,
		.error = 0
	};

	enum opt expect = OPT_NONE;

	for (int i = 0; ++i < argc; ) {
		char *arg = argv[i];

		// Silence warnings about missing default and enum cases
		diag(push)
		diag(ignored "-Wswitch")
		diag_clang(ignored "-Wswitch-default")

		// Ditto
		pragma_msvc(warning(push))
		pragma_msvc(warning(disable: 4062))

		switch (expect) {
		case OPT_OUTPUT:
		parse_o_arg:
			if (!*arg) {
				r.error = EINVAL;
				break;
			}
			r.have |= OPT_OUTPUT;
			r.output = arg;
			expect = OPT_NONE;
			continue;

		case OPT_THREADS:
		parse_t_arg:
			r.error = parse_u32(&r.threads, arg, 1U);
			if (r.error)
				break;
			r.have |= OPT_THREADS;
			expect = OPT_NONE;
			continue;

		case OPT_NONE:
			if (*arg != '-')
				break;

			++arg;
			if (*arg == '-') {
				++arg;
				if (!strcmp(arg, "benchmark")) {
					r.have |= OPT_BENCHMARK;
					continue;
				}
				if (!strcmp(arg, "help")) {
					r.have |= OPT_HELP;
					continue;
				}
				if (!strncmp(arg, "output",
				             sizeof "output" - 1U)) {
					arg += sizeof "output" - 1U;
					if (*arg == '=') {
						++arg;
						goto parse_o_arg;
					}
					if (!*arg) {
						expect = OPT_OUTPUT;
						continue;
					}
				} else if (!strncmp(arg, "threads",
				                    sizeof "threads" - 1U)) {
					arg += sizeof "threads" - 1U;
					if (*arg == '=') {
						++arg;
						goto parse_t_arg;
					}
					if (!*arg) {
						expect = OPT_THREADS;
						continue;
					}
				}
				break;
			}

		next_short_opt:
			switch (*arg++) {
			case 'b':
				r.have |= OPT_BENCHMARK;
				if (*arg)
					goto next_short_opt;
				continue;

			case 'h':
				r.have |= OPT_HELP;
				if (*arg)
					goto next_short_opt;
				continue;

			case 'o':
				if (*arg) {
					r.have |= OPT_OUTPUT;
					r.output = arg;
				} else {
					expect = OPT_OUTPUT;
				}
				continue;

			case 't':
				if (*arg)
					goto parse_t_arg;
				expect = OPT_THREADS;
				continue;
			}
		}

		diag(pop)
		pragma_msvc(warning(pop))

		if (!r.error)
			r.error = EINVAL;

		break;
	}

	if (!r.error && (expect || args_conflict(&r)))
		r.error = EINVAL;

	if ((r.have & OPT_HELP) || r.error)
		exit(args_help(&r, argv[0], usage));

	if (!(r.have & (OPT_BENCHMARK | OPT_OUTPUT)))
		r.output = "dbs26.bin";

	return r;
}

static int
parse_u32 (uint32_t *dst,
           char     *src,
           uint32_t  min)
{
	if (!*src)
		return EINVAL;

	errno = 0;
	char *end = src;
	int64_t n = _Generic(n
		, long: strtol
		, long long: strtoll
	)(src, &end, 0);

	int e = errno;
	if (!e) {
		if (*end)
			e = EINVAL;
		else if (n < min || n > UINT32_MAX)
			e = ERANGE;
		else
			*dst = (uint32_t)n;
	}

	return e;
}

static force_inline bool
args_conflict (struct args const *const a)
{
	#define F(x) (1U << (x))
	return !(F(a->have) & OPT_OK(F));
	#undef F
}

static int
args_help (struct args const *const a,
           char const        *const v0,
           args_usage_fn     *const fn)
{
	if (a->error)
		(void)fprintf(stderr, "%s: %s\n", v0, strerror(a->error));
	fn(stdout, v0);
	return a->error ? EXIT_FAILURE : EXIT_SUCCESS;
}
