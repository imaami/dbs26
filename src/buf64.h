/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file buf64.h
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_BUF64_H_
#define DBS26_SRC_BUF64_H_

#include "compat.h"

#include <stdint.h>
#include <stdlib.h>

_Static_assert(sizeof (uint64_t) == 8U,"");

#define BUF64_MUTABLE   UINT64_C(0x0000000000000001)
#define BUF64_REFERENCE UINT64_C(0x0000000000000002)
#define BUF64_MAX_LEN   UINT64_C(0x0000000040000000)
#define BUF64_MAX_SIZE  UINT64_C(0x0000000200000000)
#define BUF64_ATTR_MASK (BUF64_REFERENCE | BUF64_MUTABLE)
#define BUF64_SIZE_MASK UINT64_C(0x00000003fffffff8)
#define BUF64_ITER_MASK UINT64_C(0xfffffffc00000000)
#define BUF64_MASK      (BUF64_ITER_MASK | BUF64_SIZE_MASK | BUF64_ATTR_MASK)

/**
 * @brief A buffer of `uint64_t`
 */
struct buf64 {
	union {
		uint64_t const *imm;
		uint64_t       *mut;
	} data;
	uint64_t meta;
};

/**
 * @brief Create a buffer with ownership and mutability derived from argument types
 */
#define buf64(x, ...) _Generic((x), \
        default: buf64_init,        \
        uint64_t *: buf64_init_ref, \
        uint64_t const *: buf64_init_cref)((x) __VA_OPT__(, __VA_ARGS__))

/**
 * @brief Initialize as owner of newly-allocated memory
 */
static force_inline struct buf64
buf64_init (uint64_t len)
{
	return !len || len > BUF64_MAX_LEN
		? (struct buf64){0}
		: (struct buf64){
			.data.mut = calloc(len, sizeof (uint64_t)),
			.meta = (len << 3U) | BUF64_MUTABLE
		};
}

/**
 * @brief Check if buffer owns its memory
 *
 * @param b Buffer
 * @return true if buffer owns its memory, false otherwise
 */
static force_inline bool
buf64_owns_memory (struct buf64 const *b)
{
	return (b->meta & BUF64_ATTR_MASK) == BUF64_MUTABLE;
}

/**
 * @brief Free owned memory (if any) and uninitialize
 */
static force_inline void
buf64_fini (struct buf64 *b)
{
	if (b) {
		if (buf64_owns_memory(b))
			free(b->data.mut);
		*b = (struct buf64){0};
	}
}

/**
 * @brief Initialize as reference to mutable external memory
 */
static force_inline struct buf64
buf64_init_ref (uint64_t *ptr,
                uint64_t  len)
{
	return !ptr || len > BUF64_MAX_LEN
		? (struct buf64){0}
		: (struct buf64){
			.data.mut = ptr,
			.meta = (len << 3U) | BUF64_REFERENCE | BUF64_MUTABLE
		};
}

/**
 * @brief Initialize as reference to immutable external memory
 */
static force_inline struct buf64
buf64_init_cref (uint64_t const *ptr,
                 uint64_t        len)
{
	return !ptr || len > BUF64_MAX_LEN
		? (struct buf64){0}
		: (struct buf64){
			.data.imm = ptr,
			.meta = (len << 3U) | BUF64_REFERENCE
		};
}

/**
 * @brief Get view of existing buffer
 *
 * The returned buffer never claims ownership of the source buffer's
 * memory. Calling @ref buf64_fini() on the view will not affect the
 * source buffer's memory.
 *
 * The mutability of the returned buffer is the same as the original
 * buffer; that is, if the original buffer is mutable, the view will
 * be mutable as well.
 *
 * @param b Source buffer
 * @return View of the source buffer
 */
static force_inline struct buf64
buf64_view (struct buf64 const *b)
{
	return (struct buf64){
		.data = b->data,
		.meta = (b->meta & BUF64_MASK) | BUF64_REFERENCE
	};
}

/**
 * @brief Take ownership of buffer
 *
 * After the call, the source buffer will be uninitialized.
 *
 * @param src Source buffer
 * @return The original buffer, now owned by the caller
 */
static force_inline struct buf64
buf64_move (struct buf64 *src)
{
	struct buf64 b = *src;
	*src = (struct buf64){0};
	return b;
}

/**
 * @brief Get data size in bytes
 */
static force_inline uint64_t
buf64_size (struct buf64 const *const b)
{
	return b->meta & BUF64_SIZE_MASK;
}

/**
 * @brief Get data length in units of `uint64_t`
 */
static force_inline uint64_t
buf64_len (struct buf64 const *const b)
{
	return buf64_size(b) >> 3U;
}

/**
 * @brief Get pointer to immutable data
 */
static force_inline uint64_t const *
buf64_cdata (struct buf64 const *b)
{
	return b->data.imm;
}

/**
 * @brief Get pointer to mutable data, if permitted
 *
 * @param b Buffer
 * @return Data pointer if buffer is mutable, nullptr otherwise
 */
static force_inline uint64_t *
buf64_data (struct buf64 const *const b)
{
	return (b->meta & BUF64_MUTABLE) ? b->data.mut : nullptr;
}

#undef BUF64_MASK
#undef BUF64_ATTR_MASK
#undef BUF64_SIZE_MASK
#undef BUF64_MAX
#undef BUF64_REFERENCE
#undef BUF64_MUTABLE

#endif /* DBS26_SRC_BUF64_H_ */
