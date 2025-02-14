/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file bits.h
 * @brief Bitwise operations
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_BITS_H_
#define DBS26_SRC_BITS_H_

#include "compat.h"

#include <limits.h>
#include <stdint.h>

#ifdef _MSC_VER
# include <intrin.h>

# define return_run1(s,x)       \
  unsigned long pos = 0;        \
  return run1_##s(&pos,x)

# define run1_msb(p,x) (\
  bs_(Reverse,x)        \
     (p,cmpl(x))        \
  ? (unsigned)CHAR_BIT *\
    (unsigned)sizeof (x)\
    - 1U -(unsigned)*(p)\
  : (unsigned)CHAR_BIT *\
    (unsigned)sizeof (x))

# define run1_lsb(p,x) (\
  bs_(Forward,x)        \
     (p,cmpl(x))        \
  ? (unsigned)*(p)      \
  : (unsigned)CHAR_BIT *\
    (unsigned)sizeof (x))

# define bs_(f,x) _Generic((x)  \
  , uint32_t: _BitScan##f       \
  , uint64_t: _BitScan##f##64)

pragma_msvc(intrinsic(_BitScanForward))
pragma_msvc(intrinsic(_BitScanReverse))
pragma_msvc(intrinsic(_BitScanForward64))
pragma_msvc(intrinsic(_BitScanReverse64))

#else // _MSC_VER
# define return_run1(s,x)       \
  return (max_hamming(x)        \
          ? (unsigned)CHAR_BIT* \
            (unsigned)sizeof(x) \
          : (unsigned)cz_##s(x) \
                     (cmpl(x)))


# define max_hamming(x) (       \
  (x) == _Generic(              \
    (x), unsigned long: ~0UL,   \
    long: ~0L, unsigned: ~0U,   \
    int: ~0, long long: ~0LL,   \
    unsigned long long: ~0ULL))

# define cz_msb(x) cz_(l, x)
# define cz_lsb(x) cz_(t, x)

# define cz_(y, x) _Generic((x)                                 \
  ,long long int:__builtin_c##y##zll, unsigned:__builtin_c##y##z\
  ,long int:__builtin_c##y##zl, unsigned long:__builtin_c##y##zl\
  ,int:__builtin_c##y##z, unsigned long long:__builtin_c##y##zll)
#endif // _MSC_VER

#define cmpl(x) _Generic((x)    \
 , int: (unsigned)~(x)          \
 , long: (unsigned long)~(x)    \
 , default: ~(x), long long:    \
   (unsigned long long)~(x))

static force_inline unsigned
u32_count_msb_1 (uint32_t x)
{
	return_run1(msb, x);
}

static force_inline unsigned
u32_count_lsb_1 (uint32_t x)
{
	return_run1(lsb, x);
}

static force_inline unsigned
u64_count_msb_1 (uint64_t x)
{
	return_run1(msb, x);
}

static force_inline unsigned
u64_count_lsb_1 (uint64_t x)
{
	return_run1(lsb, x);
}

#undef cmpl
#undef return_run1

#ifdef _MSC_VER
# undef bs_
# undef run1_lsb
# undef run1_msb
#else // _MSC_VER
# undef cz_
# undef cz_lsb
# undef cz_msb
# undef max_hamming
#endif // _MSC_VER

#define count_msb_1(x) _Generic((x)     \
 ,uint32_t:u32_count_msb_1(x)           \
 ,uint64_t:u64_count_msb_1(x)           \
 ,int32_t:u32_count_msb_1((uint32_t)(x))\
 ,int64_t:u64_count_msb_1((uint64_t)(x)))

#define count_lsb_1(x) _Generic((x)     \
 ,uint32_t:u32_count_lsb_1(x)           \
 ,uint64_t:u64_count_lsb_1(x)           \
 ,int32_t:u32_count_lsb_1((uint32_t)(x))\
 ,int64_t:u64_count_lsb_1((uint64_t)(x)))

#endif /* DBS26_SRC_BITS_H_ */
