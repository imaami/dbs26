/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file solver.c
 * @author Juuso Alasuutari
 */

#include "compat.h"

#include <errno.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
# include <unistd.h>
#else
# include <Windows.h>

# include <fcntl.h>
# include <io.h>
# include <profileapi.h>
#endif

#include "nproc.h"
#include "solver.h"
#include "task.h"
#include "timer.h"
#include "worker.h"

#define countof(x) (sizeof (x) / sizeof (x)[0])

static const uint8_t task_seq_pfx[] = {

	                        0x0c, 0x0d, 0x0e, 0x0f,
	                        0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	      0x21, 0x22, 0x23,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,       0x3f,
	                  0x43, 0x44,       0x46, 0x47,
	0x48, 0x49,       0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	      0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c,       0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b,             0x7e,
	                        0x84, 0x85,
	0x88, 0x89, 0x8a, 0x8b,             0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
	            0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	      0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9,       0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5,
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,       0xbf,
	            0xc2,       0xc4, 0xc5, 0xc6,
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd,       0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc,       0xde, 0xdf,
	      0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,

	0xf8, 0xf9, 0xfa, 0xfb
};

static const uint16_t task_seq_cnt[] = {

	                        3712, 4224, 3968, 3968,
	                        2304, 5376, 4096, 4096,
	1920, 2560, 1776, 2448, 1984, 2368, 2176, 2176,
	      5632, 5632, 6144,
	1536, 2048, 3584, 4096, 2240, 3392, 2816, 2816,
	1616, 3120, 2432, 3328, 2072, 2504, 2536, 3384,
	2176, 2432, 2776, 3112, 2304, 2944,       5248,
	                  2560, 2560,       1792, 1792,
	1536, 1536,       3584, 1280, 1280, 1280, 1280,
	2560, 6144, 6656, 5120,
	2240, 2880, 3584, 3584, 3072, 3072, 3072, 3072,
	      2068, 2068, 1688, 1808, 1688, 1904, 2088,
	1712, 2640, 3584, 1792, 4096,       2816, 2816,
	1267, 2301, 2150, 2474, 1536, 1920, 2688, 2048,
	1784, 2312, 1728, 2368,             8192,
	                        4032, 4608,
	1920, 3264, 2544, 2640,             4320, 4320,
	1616, 3120, 2432, 2304, 1272, 2760, 1944, 2088,
	            4608, 5760, 2208, 2784, 2496, 2496,
	      2068, 1688, 2068, 2032, 2152, 1688, 1616,
	2240, 2880,       5120,  768, 1792, 1280, 1280,
	 832, 1856, 2176, 2304, 4096, 4608,
	2296, 2952, 2688, 2560, 2624, 2624,       5248,
	            3600,       2160, 2160, 3600,
	2176, 2176, 1904, 1904, 2336, 2336,       4320,
	1267, 2301, 2586, 2038, 3072, 3072, 1536, 1536,
	2296, 2952, 2944, 2944, 3968,       2176, 2176,
	      1800, 2160, 1800, 2176, 1904, 2336, 2160,
	1784, 2312, 3072, 1536, 2624, 2944, 2176, 1984,

	5760, 8576, 8704, 9728
};

static const uint64_t task_seq_map[] = {
	0x000000030001115f, 0x000000030001215f, 0x000000030001419f, 0x000000030001819f,
	0x0000000500120537, 0x0000000500220537, 0x0000000500420937, 0x0000000500820937,
	0x0000000901021157, 0x0000000902021157, 0x0000000904022157, 0x0000000908022157,
	0x0000000910024197, 0x0000000920024197, 0x0000000940028197, 0x0000000980028197,
	0x0000001300050317, 0x0000001500060317, 0x0000001900060317, 0x0000012100140617,
	0x0000022100140617, 0x0000042100240617, 0x0000082100240617, 0x0000102100440a17,
	0x0000202100440a17, 0x0000402100840a17, 0x0000802100840a17, 0x0001004101081217,
	0x0002004101081217, 0x0004004102081217, 0x0008004102081217, 0x0010004104082217,
	0x0020004104082217, 0x0040004108082217, 0x0080004108082217, 0x0100008110084217,
	0x0200008110084217, 0x0400008120084217, 0x0800008120084217, 0x1000008140088217,
	0x2000008140088217, 0x8000008180088217, 0x000001030011042f, 0x0000010500120437,
	0x0000010900120467, 0x00000109001204a7, 0x0000021100140527, 0x0000021100140627,
	0x0000022100140c27, 0x0000024100181427, 0x0000024100182427, 0x0000028100184427,
	0x0000028100188427, 0x0000050100310427, 0x0000050100320427, 0x0000060100340427,
	0x0000060100380427, 0x0000180101600427, 0x0000180102600427, 0x0000280104600427,
	0x0000280108600427, 0x0000480110a00427, 0x0000480120a00427, 0x0000880140a00427,
	0x0000880180a00427, 0x0001100301400827, 0x0002100501400827, 0x0002100901400827,
	0x0004101102400827, 0x0004102102400827, 0x0008104102400827, 0x0008108102400827,
	0x0010210104400827, 0x0010220104400827, 0x0020240104400827, 0x0020280104400827,
	0x0040300108400827, 0x0080600108400827, 0x0080a00108400827, 0x0101400110800827,
	0x0102400110800827, 0x0204400110800827, 0x0208400110800827, 0x0410400120800827,
	0x0420400120800827, 0x0840400120800827, 0x0880400120800827, 0x1100800140800827,
	0x1200800140800827, 0x2400800140800827, 0x2800800140800827, 0xc000800180800827,
	0x000100030100105f, 0x000100030100106f, 0x000200050100115b, 0x000200050100125b,
	0x000200050100146b, 0x000200050100186b, 0x00020009010050cb, 0x00020009010090cb,
	0x000400110201114b, 0x000400110202114b, 0x000400110204124b, 0x000400110208124b,
	0x000400210210144b, 0x000400210220144b, 0x000400210240184b, 0x000400210280184b,
	0x000800410600304b, 0x000800410a00304b, 0x000800811200504b, 0x000800812200504b,
	0x000800814200904b, 0x000800818200904b, 0x001001030401204b, 0x001001050402204b,
	0x001001090402204b, 0x001002110404204b, 0x001002210404204b, 0x001002410408204b,
	0x001002810408204b, 0x002005010410204b, 0x002006010410204b, 0x00200c010420204b,
	0x002018010440204b, 0x002028010440204b, 0x002048010480204b, 0x002088010480204b,
	0x004110010900204b, 0x004210010900204b, 0x004410010a00204b, 0x004810010a00204b,
	0x005020010c00204b, 0x006020010c00204b, 0x018040011800204b, 0x028040011800204b,
	0x048040012800204b, 0x088040012800204b, 0x108080014800204b, 0x208080014800204b,
	0x808080018800204b, 0x010100031000408f, 0x010200051000409b, 0x01020005100040ab,
	0x01020009100040cb, 0x020400111000418b, 0x020400111000428b, 0x020400211000448b,
	0x020400211000488b, 0x020800411000508b, 0x020800411000608b, 0x020800811000c08b,
	0x041001012001408b, 0x041001012002408b, 0x041002012004408b, 0x041002012008408b,
	0x042004012010408b, 0x042004012020408b, 0x042008012040408b, 0x042008012080408b,
	0x084010012100408b, 0x084010012200408b, 0x084020012400408b, 0x084020012800408b,
	0x088040013000408b, 0x088080016000408b, 0x08808001a000408b, 0x110100034000808b,
	0x110200054000808b, 0x110200094000808b, 0x120400114000808b, 0x120400214000808b,
	0x120800414000808b, 0x120800814000808b, 0x241001014000808b, 0x241002014000808b,
	0x242004014000808b, 0x242008014000808b, 0x284010014000808b, 0x284020014000808b,
	0x288040014000808b, 0x288080014000808b, 0xd10000018000808b, 0xd20000018000808b,
	0xe40000018000808b, 0xe80000018000808b
};

// Silence flexible array member warning
pragma_msvc(warning(push))
pragma_msvc(warning(disable: 4200))

struct solver {
	struct buf64     tasks[countof(task_seq_cnt)];
	_Atomic(int32_t) task_iter;
	uint32_t         n_workers;
	struct worker    workers[];
};

pragma_msvc(warning(pop))

struct solver *
solver_create (uint32_t  n_workers,
               int      *err)
{
	if (!n_workers)
		n_workers = nproc();

	(void)fprintf(stderr, "Using %" PRIu32 " threads\n", n_workers);

	struct solver *s = calloc(1U, offsetof(struct solver,
	                                       workers[n_workers]));
	if (!s) {
		if (err)
			*err = errno ? errno : ENOMEM;
		return nullptr;
	}

	atomic_init(&s->task_iter, -(int32_t)countof(s->tasks));
	s->n_workers = n_workers;

	return s;
}

static void
solver_free_tasks (struct solver *s)
{
	for (size_t i = 0U; i < countof(s->tasks); ++i) {
		buf64_fini(&s->tasks[i]);
	}
}

void
solver_destroy (struct solver **pp)
{
	if (pp) {
		struct solver *s = *pp;
		*pp = nullptr;
		if (s) {
			solver_free_tasks(s);
			free(s);
		}
	}
}

static uintptr_t
solver_cb (void *ctx)
{
	struct solver *s = ctx;
	struct task tsk = {0};
	uintptr_t count = 0U;

	for (int32_t i;
	     0 > (i = atomic_fetch_add_explicit(&s->task_iter, 1,
	                                        memory_order_relaxed));)
	{
		uint32_t const id = (uint32_t)~i;
		s->tasks[id] = task_solve(&tsk, nullptr, task_seq_map[id],
		                          (uint32_t)task_seq_cnt[id] << 7U,
		                          (uint16_t)(0x8100U | task_seq_pfx[id]));
		count += buf64_len(&s->tasks[id]);
		if (i == -1)
			break;
	}

	return count;
}

static uint32_t
solver_start_workers (struct solver *s)
{
	uint32_t i = 0U;

	for (uint32_t n = s->n_workers; i < n; ) {
		int e = worker_start(&s->workers[i], solver_cb, s);
		if (!e) {
			++i;
			continue;
		}
		--n;
		(void)fprintf(stderr, "worker_start: %s\n", strerror(e));
	}

	return i;
}

static uintptr_t
solver_wait_workers (struct solver *s,
                     uint32_t       n)
{
	uintptr_t count = 0;
	for (uint32_t i = 0; i < n; ++i) {
		int e = worker_wait(&s->workers[i]);
		count += !e * s->workers[i].ret;
	}
	return count;
}

void
solver_solve (struct solver *s,
              char const    *out)
{
	timestamp t = now();
	uint32_t n_workers = solver_start_workers(s);
	if (!n_workers)
		return;

	uintptr_t seq_count = solver_wait_workers(s, n_workers);
	double ms = since(t);

	(void)fprintf(stderr, "Generated %" PRIuPTR " sequences"
	              " in %.3lf ms\n", seq_count, ms);

	FILE *f = nullptr;

	if (out && seq_count == 67108864U) {
		if (out[0] == '-' && !out[1]) {
			f = stdout;
			out = nullptr;
#ifdef _WIN32
			(void)fflush(stdout);
			(void)_setmode(_fileno(stdout), _O_BINARY);
#endif
		} else {
#ifndef _WIN32
			f = fopen(out, "wbe");
			if (!f &&
			    (errno != EINVAL || !(f = fopen(out, "wb"))))
#else
			f = fopen(out, "wb");
			if (!f)
#endif
				perror("fopen");
			else
				(void)fprintf(stderr, "Saving to %s\n", out);
		}
	}

	for (size_t i = 0U; i < countof(s->tasks); ++i) {
		size_t len = buf64_len(&s->tasks[i]);
		if (len && f) {
			uint64_t const *ptr = buf64_cdata(&s->tasks[i]);
			if (fwrite(ptr, sizeof *ptr, len, f) != len) {
				if (out) {
					perror("fwrite");
					(void)fclose(f);
				}
				f = nullptr;
			}
		}
	}

	if (f && out)
		(void)fclose(f);
}
