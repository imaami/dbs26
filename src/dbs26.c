/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file dbs26.c
 * @brief Generate all 67108864 unique binary De Bruijn sequences
 *        with subsequence length 6, ordered by value.
 * @author Juuso Alasuutari
 */

#ifdef __cplusplus
# error "This is C and won't compile in C++ mode."
#endif

#ifdef _WIN32
# define _CRT_SECURE_NO_WARNINGS
# define WIN32_LEAN_AND_MEAN
#endif

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
# include <pthread.h>
# include <time.h>
# include <unistd.h>
#else
# include <Windows.h>

# include <fcntl.h>
# include <io.h>
# include <process.h>
# include <profileapi.h>
#endif

#ifdef _MSC_VER
# include <intrin.h>
#endif

#undef HAVE_C23_BOOL
#undef HAVE_C23_NULLPTR

#if __STDC_VERSION__ >= 202000L && !defined __INTELLISENSE__
# ifdef __clang_major__
#  if __clang_major__ >= 15
#   define HAVE_C23_BOOL
#  endif
#  if __clang_major__ >= 16
#   define HAVE_C23_NULLPTR
#  endif
# elif defined __GNUC__
#  if __GNUC__ > 13 || (__GNUC__ == 13 && __GNUC_MINOR__ >= 1)
#   define HAVE_C23_BOOL
#   define HAVE_C23_NULLPTR
#  endif
# endif
#endif

#ifndef HAVE_C23_BOOL
# include <stdbool.h>
#endif

#ifndef HAVE_C23_NULLPTR
# define nullptr NULL
#endif

#ifdef __clang__
# pragma clang diagnostic ignored "-Wunknown-warning-option"
# pragma clang diagnostic ignored "-Wcast-align"
# pragma clang diagnostic ignored "-Wdeclaration-after-statement"
# pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
# pragma clang diagnostic ignored "-Wpre-c11-compat"
# pragma clang diagnostic ignored "-Wpre-c23-compat"
# pragma clang diagnostic ignored "-Wpre-c2x-compat"
# pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

#define container_of(ptr, T, member) ((T *)( \
  (unsigned char *)(1 ? (ptr) : &((T *)0)->member) - offsetof(T, member) \
))

#define countof(x) (sizeof (x) / sizeof (x)[0])

#ifndef _MSC_VER
# define force_inline __attribute__((always_inline)) inline
# define const_inline __attribute__((const)) force_inline

# define all_bits_set(x) (_Generic((x) \
  , int: ~0                            \
  , long: ~0L                          \
  , long long: ~0LL                    \
  , unsigned int: ~0U                  \
  , unsigned long: ~0UL                \
  , unsigned long long: ~0ULL) == (x))

# define pick_clz(x) _Generic((x)      \
  , int: __builtin_clz                 \
  , long: __builtin_clzl               \
  , long long: __builtin_clzll         \
  , unsigned int: __builtin_clz        \
  , unsigned long: __builtin_clzl      \
  , unsigned long long: __builtin_clzll)

# define pick_ctz(x) _Generic((x)      \
  , int: __builtin_ctz                 \
  , long: __builtin_ctzl               \
  , long long: __builtin_ctzll         \
  , unsigned int: __builtin_ctz        \
  , unsigned long: __builtin_ctzl      \
  , unsigned long long: __builtin_ctzll)

# define unsigned_not(x) _Generic((x)  \
  , default: ~(x)                      \
  , int: (unsigned int)~(x)            \
  , long: (unsigned long)~(x)          \
  , long long: (unsigned long long)~(x))

# define count_msb_1(x) (unsigned)(all_bits_set(x) \
  ? (int)sizeof(x) * CHAR_BIT : pick_clz(x)(unsigned_not(x)))
# define count_lsb_1(x) (unsigned)(all_bits_set(x) \
  ? (int)sizeof(x) * CHAR_BIT : pick_ctz(x)(unsigned_not(x)))

#else
# define force_inline __forceinline
# define const_inline __forceinline

# define count_msb_1(x) _Generic((x)        \
  , uint32_t: u32_count_msb_1(x)            \
  , uint64_t: u64_count_msb_1(x)            \
  , int32_t: u32_count_msb_1((uint32_t)(x)) \
  , int64_t: u64_count_msb_1((uint64_t)(x)))

# define count_lsb_1(x) _Generic((x)        \
  , uint32_t: u32_count_lsb_1(x)            \
  , uint64_t: u64_count_lsb_1(x)            \
  , int32_t: u32_count_lsb_1((uint32_t)(x)) \
  , int64_t: u64_count_lsb_1((uint64_t)(x)))

# pragma intrinsic(_BitScanForward)
# pragma intrinsic(_BitScanReverse)
# pragma intrinsic(_BitScanForward64)
# pragma intrinsic(_BitScanReverse64)

static force_inline unsigned
u32_count_msb_1 (uint32_t x)
{
	unsigned long pos = 0;
	return _BitScanReverse(&pos, ~x) ? 31U - (unsigned)pos : 32U;
}

static force_inline unsigned
u32_count_lsb_1 (uint32_t x)
{
	unsigned long pos = 0;
	return _BitScanForward(&pos, ~x) ? (unsigned)pos : 32U;
}

static force_inline unsigned
u64_count_msb_1 (uint64_t x)
{
	unsigned long pos = 0;
	return _BitScanReverse64(&pos, ~x) ? 63U - (unsigned)pos : 64U;
}

static force_inline unsigned
u64_count_lsb_1 (uint64_t x)
{
	unsigned long pos = 0;
	return _BitScanForward64(&pos, ~x) ? (unsigned)pos : 64U;
}
#endif

#define SUB_LEN 6U
#define SEQ_LEN (1U << SUB_LEN)
#define SUB_LAST (SEQ_LEN - 1U)
#define SUB_MASK (SEQ_LEN - 1U)

#define SEARCH_SPACE(n) ((1U << n) - n - 2U)
#define SEARCH_DEPTH(n) (SEARCH_SPACE(n) / n)

#if 0
static const_inline struct s16 {
	char d[16U + 1U];
} u64_hex_str (uint64_t v) {
	struct s16 r = { "0000000000000000" };
	const char x[16] = "0123456789abcdef";
	for (int i = 16; v; v >>= 4U)
		r.d[--i] = x[v & 15U];
	return r;
}
#endif

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

struct u64_pair {
	uint64_t end;
	uint64_t map;
};

struct stk {
	uint32_t        sp;
	uint32_t        sum;
	//uint64_t        seq;
	struct u64_pair stk[SEARCH_DEPTH(SUB_LEN) - 1U];
};

static uint32_t
scan (struct stk *stk,
      uint64_t   *dst,
      uint64_t    seq,
      uint64_t    map);

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
scan6 (struct stk      *stk,
       uint64_t *const  dst,
       uint64_t         seq)
{
	uint64_t const end = stk->stk[stk->sp].end;
	uint64_t const map = stk->stk[stk->sp].map;
	uint32_t n = 0U;

	for (stk->sp++;; seq++) {
		uint64_t m = validate_map(seq, map, SUB_LEN);
		if (m) {
			n += scan(stk, &dst[n], seq, m);
		}

		if (end == seq)
			break;
	}

	stk->sp--;
	return n;
}

static force_inline uint32_t
scan5 (struct stk const *const stk,
       uint64_t *const         dst,
       uint64_t                seq)
{
	uint64_t const end = stk->stk[stk->sp].end;
	uint64_t const map = stk->stk[stk->sp].map;
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
scan (struct stk      *stk,
      uint64_t *const  dst,
      uint64_t         seq,
      uint64_t         map)
{
	seq <<= SUB_LEN;
	stk->stk[stk->sp].end = seq + SUB_LAST - count_msb_1(map);
	stk->stk[stk->sp].map = map;
	seq += count_lsb_1(map);
	return stk->sp < countof(stk->stk) - 1U
	       ? scan6(stk, dst, seq)
	       : scan5(stk, dst, seq);
}

struct worker {
	size_t    id;
#ifndef _WIN32
	pthread_t tid;
#else
	uintptr_t tid;
#endif
};

#ifndef _WIN32
typedef void *worker_func_t(void *);
#else
typedef unsigned __stdcall worker_func_t(void *);
#endif

static int
worker_start (struct worker *w,
              worker_func_t *f)
{
#ifndef _WIN32
	return pthread_create(&w->tid, nullptr, f, w);
#else
	w->tid = _beginthreadex(nullptr, 0, f, w, 0, nullptr);
	return w->tid ? 0 : errno;
#endif
}

static uintptr_t
worker_wait (struct worker *w)
{
	uintptr_t seq_count = 0U;

#ifndef _WIN32
	void *r = nullptr;
	int e = pthread_join(w->tid, &r);
	if (e)
		(void)fprintf(stderr, "pthread_join: %s\n", strerror(e));
# ifndef __ANDROID__
	else if (r != PTHREAD_CANCELED)
# else // __ANDROID__
	else
# endif // __ANDROID__
		seq_count = (uintptr_t)r;
#else
	DWORD r = WaitForSingleObject((HANDLE)w->tid, INFINITE);
	if (r == WAIT_OBJECT_0) {
		r = 0;
		if (GetExitCodeThread((HANDLE)w->tid, &r))
			seq_count = (uintptr_t)r;
	}
	CloseHandle((HANDLE)w->tid);
	w->tid = 0;
#endif

	return seq_count;
}

/**
 * @brief A view into an array of 64-bit integers.
 *
 * @note The members are _arrays_ of pointers, not simple pointer
 *       variables, to leverage C's implicit initialization rules
 *       for initializer lists and compound literals. This way we
 *       can put 32-bit pointers into 64-bit fields and guarantee
 *       that no space between them is left uninitialized. 64-bit
 *       systems get one-element arrays, while 32-bit systems get
 *       arrays of two elements where the second is considered as
 *       padding.
 */
struct u64_view {
	uint64_t *begin[sizeof(uint64_t) / sizeof(uint64_t *)];
	uint64_t *end[sizeof(uint64_t) / sizeof(uint64_t *)];
};

static force_inline struct u64_view
u64_view (uint64_t *const begin,
          uint64_t *const end)
{
	return (struct u64_view){
		.begin = {begin},
		.end = {end}
	};
}

static force_inline size_t
u64_view_len (struct u64_view const view)
{
	return (size_t)(view.end[0] - view.begin[0]);
}

static const uint16_t task_seq_prefix[] = {
	0x810c, 0x810d, 0x810e, 0x810f, 0x8114, 0x8115, 0x8116, 0x8117,
	0x8118, 0x8119, 0x811a, 0x811b, 0x811c, 0x811d, 0x811e, 0x811f,
	0x8121, 0x8122, 0x8123, 0x8128, 0x8129, 0x812a, 0x812b, 0x812c,
	0x812d, 0x812e, 0x812f, 0x8130, 0x8131, 0x8132, 0x8133, 0x8134,
	0x8135, 0x8136, 0x8137, 0x8138, 0x8139, 0x813a, 0x813b, 0x813c,
	0x813d, 0x813f, 0x8143, 0x8144, 0x8146, 0x8147, 0x8148, 0x8149,
	0x814b, 0x814c, 0x814d, 0x814e, 0x814f, 0x8150, 0x8151, 0x8152,
	0x8153, 0x8158, 0x8159, 0x815a, 0x815b, 0x815c, 0x815d, 0x815e,
	0x815f, 0x8161, 0x8162, 0x8163, 0x8164, 0x8165, 0x8166, 0x8167,
	0x8168, 0x8169, 0x816a, 0x816b, 0x816c, 0x816e, 0x816f, 0x8170,
	0x8171, 0x8172, 0x8173, 0x8174, 0x8175, 0x8176, 0x8177, 0x8178,
	0x8179, 0x817a, 0x817b, 0x817e, 0x8184, 0x8185, 0x8188, 0x8189,
	0x818a, 0x818b, 0x818e, 0x818f, 0x8190, 0x8191, 0x8192, 0x8193,
	0x8194, 0x8195, 0x8196, 0x8197, 0x819a, 0x819b, 0x819c, 0x819d,
	0x819e, 0x819f, 0x81a1, 0x81a2, 0x81a3, 0x81a4, 0x81a5, 0x81a6,
	0x81a7, 0x81a8, 0x81a9, 0x81ab, 0x81ac, 0x81ad, 0x81ae, 0x81af,
	0x81b0, 0x81b1, 0x81b2, 0x81b3, 0x81b4, 0x81b5, 0x81b8, 0x81b9,
	0x81ba, 0x81bb, 0x81bc, 0x81bd, 0x81bf, 0x81c2, 0x81c4, 0x81c5,
	0x81c6, 0x81c8, 0x81c9, 0x81ca, 0x81cb, 0x81cc, 0x81cd, 0x81cf,
	0x81d0, 0x81d1, 0x81d2, 0x81d3, 0x81d4, 0x81d5, 0x81d6, 0x81d7,
	0x81d8, 0x81d9, 0x81da, 0x81db, 0x81dc, 0x81de, 0x81df, 0x81e1,
	0x81e2, 0x81e3, 0x81e4, 0x81e5, 0x81e6, 0x81e7, 0x81e8, 0x81e9,
	0x81ea, 0x81eb, 0x81ec, 0x81ed, 0x81ee, 0x81ef, 0x81f8, 0x81f9,
	0x81fa, 0x81fb
};

static const uint32_t task_seq_count[] = {
	 475136,  540672,  507904,  507904,  294912,  688128,  524288,  524288,
	 245760,  327680,  227328,  313344,  253952,  303104,  278528,  278528,
	 720896,  720896,  786432,  196608,  262144,  458752,  524288,  286720,
	 434176,  360448,  360448,  206848,  399360,  311296,  425984,  265216,
	 320512,  324608,  433152,  278528,  311296,  355328,  398336,  294912,
	 376832,  671744,  327680,  327680,  229376,  229376,  196608,  196608,
	 458752,  163840,  163840,  163840,  163840,  327680,  786432,  851968,
	 655360,  286720,  368640,  458752,  458752,  393216,  393216,  393216,
	 393216,  264704,  264704,  216064,  231424,  216064,  243712,  267264,
	 219136,  337920,  458752,  229376,  524288,  360448,  360448,  162176,
	 294528,  275200,  316672,  196608,  245760,  344064,  262144,  228352,
	 295936,  221184,  303104, 1048576,  516096,  589824,  245760,  417792,
	 325632,  337920,  552960,  552960,  206848,  399360,  311296,  294912,
	 162816,  353280,  248832,  267264,  589824,  737280,  282624,  356352,
	 319488,  319488,  264704,  216064,  264704,  260096,  275456,  216064,
	 206848,  286720,  368640,  655360,   98304,  229376,  163840,  163840,
	 106496,  237568,  278528,  294912,  524288,  589824,  293888,  377856,
	 344064,  327680,  335872,  335872,  671744,  460800,  276480,  276480,
	 460800,  278528,  278528,  243712,  243712,  299008,  299008,  552960,
	 162176,  294528,  331008,  260864,  393216,  393216,  196608,  196608,
	 293888,  377856,  376832,  376832,  507904,  278528,  278528,  230400,
	 276480,  230400,  278528,  243712,  299008,  276480,  228352,  295936,
	 393216,  196608,  335872,  376832,  278528,  253952,  737280, 1097728,
	1114112, 1245184
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

struct solver {
	struct u64_view   tasks[countof(task_seq_count)];
	_Atomic(int32_t)  task_iter;
	uint32_t          n_workers;
	struct worker     workers[];
};

static struct u64_view
task_solve (struct stk *const stk,
            uint32_t const    id)
{
	uint64_t *dst = malloc(task_seq_count[id] * sizeof *dst);
	if (dst) {
		stk->sp = 0U;
		uint32_t n = scan(stk, dst, task_seq_prefix[id],
		                  task_seq_map[id]);
		if (task_seq_count[id] == n)
			return u64_view(dst, dst + n);
		free(dst);
	}
	return u64_view(nullptr, nullptr);
}

static void
solver_destroy (struct solver **pp);

static uint32_t
nproc (void)
{
#ifndef _WIN32
# ifdef _SC_NPROCESSORS_ONLN
	long n = sysconf(_SC_NPROCESSORS_ONLN);
	if (n > 0L) {
#  if LONG_MAX > UINT32_MAX
		if (n > (long)UINT32_MAX)
			n = (long)UINT32_MAX;
#  endif
		return (uint32_t)n;
	}
# endif // _SC_NPROCESSORS_ONLN
#else // _WIN32
	DWORD n = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	if (n > 0)
		return (uint32_t)n;
#endif // _WIN32
	return 1U;
}

enum opt {
	OPT_NONE      = 0U,
	OPT_OUTPUT    = 1U << 0U,
	OPT_THREADS   = 1U << 1U,
	OPT_BENCHMARK = 1U << 2U,
	OPT_HELP      = 1U << 3U,
};

struct args {
	uintptr_t   have;
	char const *output;
	uint32_t    threads;
	int32_t     error;
};

#define F(x) (1U << (x))
#define ALLOWED_OPT_COMBOS                                   \
 ( F(OPT_NONE                                              ) \
 | F(         OPT_OUTPUT                                   ) \
 | F(                    OPT_THREADS                       ) \
 | F(         OPT_OUTPUT|OPT_THREADS                       ) \
 | F(                                OPT_BENCHMARK         ) \
 | F(                    OPT_THREADS|OPT_BENCHMARK         ) \
 | F(                                              OPT_HELP) )

static force_inline bool
args_conflict (struct args const *const a)
{
	return !(F(a->have) & ALLOWED_OPT_COMBOS);
}

#undef F
#undef ALLOWED_OPT_COMBOS

static int
parse_uint32 (uint32_t *dest,
              char     *src)
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
		else if (n < 0 || n > UINT32_MAX)
			e = ERANGE;
		else
			*dest = (uint32_t)n;
	}

	return e;
}

static int
help (char const *argv0,
      int         error);

static struct args
args (int   argc,
      char *argv[])
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

		#ifdef __clang__
		# pragma clang diagnostic push
		# pragma clang diagnostic ignored "-Wswitch"
		# pragma clang diagnostic ignored "-Wswitch-default"
		#elif defined __GNUC__
		# pragma GCC diagnostic push
		# pragma GCC diagnostic ignored "-Wswitch"
		#endif

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
			r.error = parse_uint32(&r.threads, arg);
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

		#ifdef __clang__
		# pragma clang diagnostic pop
		#elif defined __GNUC__
		# pragma GCC diagnostic pop
		#endif

		if (!r.error)
			r.error = EINVAL;

		break;
	}

	if (!r.error && (expect || args_conflict(&r)))
		r.error = EINVAL;

	if ((r.have & OPT_HELP) || r.error)
		exit(help(argv[0], r.error));

	return r;
}

static struct solver *
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
	for (uint32_t i = 0U; i < n_workers; ++i) {
		s->workers[i].id = i;
	}

	return s;
}

static void
solver_free_tasks (struct solver *s)
{
	for (size_t i = 0U; i < countof(s->tasks); ++i) {
		free(s->tasks[i].begin[0]);
		s->tasks[i] = u64_view(nullptr, nullptr);
	}
}

static void
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

#ifndef _WIN32
static void *
#else
static unsigned __stdcall
#endif
worker_func (void *arg)
{
	struct stk stk = {0};
	struct worker *w = arg;
	struct solver *s = container_of(w, struct solver, workers[w->id]);
	unsigned count = 0U;

	for (int32_t i;
	     0 > (i = atomic_fetch_add_explicit(&s->task_iter, 1,
	                                        memory_order_relaxed));)
	{
		uint32_t const id = (uint32_t)~i;
		s->tasks[id] = task_solve(&stk, id);
		count += u64_view_len(s->tasks[id]);
		if (i == -1)
			break;
	}

#ifndef _WIN32
	return (void *)(uintptr_t)count;
#else
	_endthreadex(count);
# ifdef _MSC_VER
	return count;
# endif // _MSC_VER
#endif // _WIN32
}

static uint32_t
solver_start_workers (struct solver *s)
{
	uint32_t i = 0U;

	for (uint32_t n = s->n_workers; i < n; ) {
		int e = worker_start(&s->workers[i], worker_func);
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
	uintptr_t seq_count = 0U;

	for (uint32_t i = 0U; i < n; ++i) {
		seq_count += worker_wait(&s->workers[i]);
	}

	return seq_count;
}

static void
solver_solve (struct solver *s,
              char const    *out)
{
#ifndef _WIN32
	struct timespec t1 = {0}, t2 = {0};
	(void)clock_gettime(CLOCK_MONOTONIC, &t1);
#else
	LARGE_INTEGER pf = {0}, t1 = {0}, t2 = {0};
	QueryPerformanceFrequency(&pf);
	QueryPerformanceCounter(&t1);
#endif
	uint32_t n_workers = solver_start_workers(s);
	if (!n_workers)
		return;

	uintptr_t seq_count = solver_wait_workers(s, n_workers);

#ifndef _WIN32
	(void)clock_gettime(CLOCK_MONOTONIC, &t2);
	double ms = (double)(t2.tv_sec - t1.tv_sec) * 1000.0
	            + (double)t2.tv_nsec / 1000000.0
	            - (double)t1.tv_nsec / 1000000.0;
#else
	QueryPerformanceCounter(&t2);
	double ms = (double)(t2.QuadPart - t1.QuadPart) * 1000.0
	            / (double)pf.QuadPart;
#endif
	(void)fprintf(stderr, "Generated %zu sequences"
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
		size_t len = u64_view_len(s->tasks[i]);
		if (len && f) {
			uint64_t const *ptr = s->tasks[i].begin[0];
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

int
main (int   argc,
      char *argv[])
{
	struct args a = args(argc, argv);

	int e = 0;
	struct solver *s = solver_create(a.threads, &e);
	if (!s) {
		(void)fprintf(stderr, "solver_create: %s\n", strerror(e));
		return EXIT_FAILURE;
	}

	if (!(a.have & (OPT_BENCHMARK | OPT_OUTPUT)))
		a.output = "dbs26.bin";

	solver_solve(s, a.output);
	solver_destroy(&s);

	return EXIT_SUCCESS;
}

static int
help (char const *argv0,
      int         error)
{
	if (error)
		(void)fprintf(stderr, "%s: %s\n", argv0, strerror(error));

	(void)fprintf(stderr,
	              "Usage: %s [-o <file>] [-t <n>]"
	              "\n       %s -b [-t <n>]"
	              "\n       %s -h"
	              "\n"
	              "\nGenerates all binary De Bruijn sequences with subsequence"
	              "\nlength 6 (all 67108864 of them)."
	              "\n"
	              "\nOptions:"
	              "\n  -h, --help            Print this help message and exit"
	              "\n  -b, --benchmark       Only benchmark, don't output data"
	              "\n  -o, --output <file>   Save output to <file> (dbs26.bin)"
	              "\n  -t, --threads <n>     Use <n> threads (available cores)"
	              "\n"
	              "\nWhen no arguments are given, computes the sequences using"
	              "\nall available logical CPUs and saves them to a file named"
	              "\ndbs26.bin in the current directory. Output data is always"
	              "\nraw binary uint64_t data in the native endianness."
	              "\n"
	              "\nSpecifying the output file as a dash ('-') will print the"
	              "\nsequences to standard output in binary mode. Only do this"
	              "\nwhen redirecting the output to a file or another program."
	              "\n"
	              "\nOn systems where xxd is available you can view the output"
	              "\nwith the following (or similar) command:"
	              "\n"
	              "\n  %s -o- | xxd -e -g8 | less"
	              "\n"
	              "\nNote: the size of the raw output is 512 MiB - be careful!"
	              "\n", argv0, argv0, argv0, argv0);

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
