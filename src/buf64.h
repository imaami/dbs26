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

#define BUF64_MAX  UINT64_C(0x0000000040000000)
#define BUF64_LEN  UINT64_C(0x000000007fffffff)
#define BUF64_LEFT UINT64_C(0x3fffffff80000000)
#define BUF64_MUT  UINT64_C(0x4000000000000000)
#define BUF64_REF  UINT64_C(0x8000000000000000)
#define BUF64_ATTR (BUF64_REF | BUF64_MUT)

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

diag_gcc(push)
diag_gcc(ignored "-Wpedantic")

/**
 * @brief Create a buffer with ownership and mutability derived from argument types
 */
#if gcc_older_than_version(15) || clang_older_than_version(12)
#define buf64(x, ...) _Generic((x), \
        default: buf64_init,        \
        uint64_t *: buf64_init_ref, \
        uint64_t const *: buf64_init_cref)((x), ## __VA_ARGS__)
#else
#define buf64(x, ...) _Generic((x), \
        default: buf64_init,        \
        uint64_t *: buf64_init_ref, \
        uint64_t const *: buf64_init_cref)((x) __VA_OPT__(, __VA_ARGS__))
#endif

diag_gcc(pop)

/**
 * @brief Initialize as owner of newly-allocated memory
 */
static force_inline struct buf64
buf64_init (uint64_t len)
{
	return !len || len > BUF64_MAX
		? (struct buf64){0}
		: (struct buf64){
			.data.mut = calloc(len, sizeof (uint64_t)),
			.meta = BUF64_MUT | (len << 31U)
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
	return (b->meta & BUF64_ATTR) == BUF64_MUT;
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
	return !ptr || len > BUF64_MAX
		? (struct buf64){0}
		: (struct buf64){
			.data.mut = ptr,
			.meta = BUF64_REF | BUF64_MUT | (len << 31U)
		};
}

/**
 * @brief Initialize as reference to immutable external memory
 */
static force_inline struct buf64
buf64_init_cref (uint64_t const *ptr,
                 uint64_t        len)
{
	return !ptr || len > BUF64_MAX
		? (struct buf64){0}
		: (struct buf64){
			.data.imm = ptr,
			.meta = BUF64_REF | (len << 31U)
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
		.meta = b->meta | BUF64_REF
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
 * @brief Get unused buffer size in units of `uint64_t`
 */
static force_inline uint64_t
buf64_left (struct buf64 const *const b)
{
	return (b->meta & BUF64_LEFT) >> 31U;
}

/**
 * @brief Check if buffer is full
 */
static force_inline bool
buf64_is_full (struct buf64 const *const b)
{
	return b->meta & BUF64_LEFT;
}

/**
 * @brief Get data length in units of `uint64_t`
 */
static force_inline uint64_t
buf64_len (struct buf64 const *const b)
{
	return (b->meta + (b->meta >> 31U)) & BUF64_LEN;
}

/**
 * @brief Get data size in bytes
 */
static force_inline uint64_t
buf64_size (struct buf64 const *const b)
{
	return buf64_len(b) * sizeof (uint64_t);
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
	return (b->meta & BUF64_MUT) ? b->data.mut : nullptr;
}

/**
 * @brief Reset the buffer position to 0
 */
static force_inline struct buf64 *
buf64_rewind (struct buf64 *const b)
{
	uint64_t len = b->meta & BUF64_LEN;
	b->meta += len * BUF64_LEN;
	return b;
}

#undef BUF64_ATTR
#undef BUF64_REF
#undef BUF64_MUT
#undef BUF64_LEFT
#undef BUF64_LEN
#undef BUF64_MAX

#endif /* DBS26_SRC_BUF64_H_ */
