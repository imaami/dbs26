/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file compiler.h
 * @brief Compiler identification convenience macros
 * @author Juuso Alasuutari
 */
#ifndef DBS26_SRC_COMPILER_H_
#define DBS26_SRC_COMPILER_H_

#ifdef __clang_major__
# define v_clang() GEN_V(__clang_major__,    \
                         __clang_minor__,    \
                         __clang_patchlevel__)
# define clang_equal_to_version(...) CMP_V_(==,clang,__VA_ARGS__)
# define clang_at_least_version(...) CMP_V_(>=,clang,__VA_ARGS__)
# define clang_newer_than_version(...) CMP_V_(>,clang,__VA_ARGS__)
# define clang_at_most_version(...) CMP_V_(<=,clang,__VA_ARGS__)
# define clang_older_than_version(...) CMP_V_(<,clang,__VA_ARGS__)
# define clang_not_version(...) CMP_V_(!=,clang,__VA_ARGS__)
#else
# define clang_equal_to_version(...) 0
# define clang_at_least_version(...) 0
# define clang_newer_than_version(...) 0
# define clang_at_most_version(...) 0
# define clang_older_than_version(...) 0
# define clang_not_version(...) 0
#endif

#if !defined __clang_major__ && defined __GNUC__
# define v_gcc() GEN_V(__GNUC__,          \
                       __GNUC_MINOR__,    \
                       __GNUC_PATCHLEVEL__)
# define gcc_equal_to_version(...) CMP_V_(==,gcc,__VA_ARGS__)
# define gcc_at_least_version(...) CMP_V_(>=,gcc,__VA_ARGS__)
# define gcc_newer_than_version(...) CMP_V_(>,gcc,__VA_ARGS__)
# define gcc_at_most_version(...) CMP_V_(<=,gcc,__VA_ARGS__)
# define gcc_older_than_version(...) CMP_V_(<,gcc,__VA_ARGS__)
# define gcc_not_version(...) CMP_V_(!=,gcc,__VA_ARGS__)
#else
# define gcc_equal_to_version(...) 0
# define gcc_at_least_version(...) 0
# define gcc_newer_than_version(...) 0
# define gcc_at_most_version(...) 0
# define gcc_older_than_version(...) 0
# define gcc_not_version(...) 0
#endif

#if defined v_clang || defined v_gcc
# define CMP_V_(op, id, ...) (v_##id() op GEN_V(__VA_ARGS__))
# define GEN_V(...) GEN_V_(__VA_ARGS__+0,0,0,)
# define GEN_V_(a, b, c, ...) \
        ( (((a)&0x3ffU)<<21U) \
        | (((b)&0x3ffU)<<11U) \
        |  ((c)&0x7ffU)       )
#endif

#endif /* DBS26_SRC_COMPILER_H_ */
