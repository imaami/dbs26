/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file task.c
 * @author Juuso Alasuutari
 */

#include "compat.h"

#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bits.h"
#include "task.h"

#define countof(x) (sizeof (x) / sizeof (x)[0])

#define SEQ_LEN (1U << SUB_LEN)
#define SUB_LAST (SEQ_LEN - 1U)
#define SUB_MASK (SEQ_LEN - 1U)

/** @brief Test a single bit in a bitmap, set it to 1 if it's currently unset,
 *         and finally return its previous value.
 *
 * @note No input validation.
 *
 * @param map Bitmap to check and possibly modify.
 * @param pos Bit position to test. Must be less than 64.
 * @return    The original state of the specified bit.
 *
 * @retval false The bit at @a pos was unset on function entry and is now set.
 * @retval true  The bit at @a pos was set on function entry and is unchanged.
 */
static force_inline bool
seen (uint64_t *const map,
      uint64_t  const pos)
{
	uint64_t const bit = UINT64_C(1) << pos;
	if (*map & bit)
		return true;
	*map |= bit;
	return false;
}

/** @brief Left-rotate a 64-bit sequence.
 *
 * @note No input validation.
 *
 * @param seq Sequence to rotate.
 * @param off Amount of rotation.
 * @return    Rotated sequence.
 */
static const_inline uint64_t
rol_64 (uint64_t const seq,
        unsigned const off)
{
	return seq << off | seq >> (64U - off);
}

static uint32_t
scan (struct task *tsk,
      uint64_t    *dst,
      uint64_t     seq,
      uint64_t     map);

static force_inline uint64_t
validate_map (uint64_t seq,
              uint64_t map,
              uint32_t num)
{
	for (; !seen(&map, seq & SUB_MASK); seq >>= 1U) {
		if (!--num)
			return map;
	}

	return 0U;
}

static force_inline uint64_t
validate_seq (uint64_t seq,
              uint64_t map,
              uint32_t num)
{
	for (; !seen(&map, seq & SUB_MASK); seq >>= 1U) {
		if (!--num)
			return seq;
	}

	return 0U;
}

static force_inline uint32_t
scan6 (struct task      *tsk,
       uint64_t *const   dst,
       uint64_t          seq)
{
	uint64_t const end = tsk->stk[tsk->sp].end;
	uint64_t const map = tsk->stk[tsk->sp].map;
	uint32_t n = 0U;

	for (tsk->sp++;; seq++) {
		uint64_t m = validate_map(seq, map, SUB_LEN);
		if (m) {
			n += scan(tsk, &dst[n], seq, m);
		}

		if (end == seq)
			break;
	}

	tsk->sp--;
	return n;
}

static force_inline uint32_t
scan5 (struct task const *const tsk,
       uint64_t          *const dst,
       uint64_t                 seq)
{
	uint64_t const end = tsk->stk[tsk->sp].end;
	uint64_t const map = tsk->stk[tsk->sp].map;
	uint32_t n = 0U;

	for (;; ++seq) {
		uint64_t m = validate_map(seq, map, SUB_LEN);
		if (m) {
			uint64_t q = validate_seq(rol_64(seq, SUB_LEN - 1U),
			                          m, SUB_LEN - 1U);
			if (q)
				dst[n++] = q;
		}

		if (end == seq)
			break;
	}

	return n;
}

static uint32_t
scan (struct task      *tsk,
      uint64_t *const   dst,
      uint64_t          seq,
      uint64_t          map)
{
	seq <<= SUB_LEN;
	tsk->stk[tsk->sp].end = seq + SUB_LAST - count_msb_1(map);
	tsk->stk[tsk->sp].map = map;
	seq += count_lsb_1(map);
	return tsk->sp < countof(tsk->stk) - 1U
	       ? scan6(tsk, dst, seq)
	       : scan5(tsk, dst, seq);
}

struct buf64
task_solve (struct task *const tsk,
            uint64_t    *const dst,
            uint64_t     const map,
            uint32_t     const cnt,
            uint16_t     const pfx)
{
	struct buf64 ret = dst ? buf64_init_ref(dst, cnt)
	                       : buf64_init(cnt);
	uint64_t *d = buf64_data(&ret);
	if (d) {
		tsk->sp = 0U;
		if (scan(tsk, d, pfx, map) != cnt)
			buf64_fini(&ret);
	}

	return ret;
}
