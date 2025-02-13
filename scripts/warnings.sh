#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-3.0-or-later
# Author: Juuso Alasuutari
#
# Lists warnings emitted by various versions of Clang and GCC.
#

run_() {
	local c o s s_="${1}_s$2[@]" v v_="${1}_v$2[@]" w
	local -i k=${#1} m n
	((k = (k < 5) * (5 - k)))
	for s in "${!s_}"; do
		((m = ${#s}, m = m < 5 ? (5 - m) : 0))
		for v in "${!v_}"; do
			c="$1-$v"
			command -v "$c" >/dev/null || continue
			((n = ${#v}, n = n < 3 ? (3 - n) + k : k))
			o=$("$c" "-std=$s" "${@:3}" -o "dbs26-$c-$s" 2>&1)
			w=$(grep -o -- '\[-W[^]][^]]*\]' <<< "$o"|sort -u)
			[[ -z "$w" ]] || {
				w="${w//[}"
				w="${w//]}"
				printf '\e[32m%s\e[m%*s \e[35m%s\e[m -std=\e[36m%s\e[m%*s' \
				       "$c" $n '' "$("$c" "-std=$s" -xc -E -dM - <<< ''    \
				                     |grep __STDC_VERSION__|cut -d_ -f6-)" \
				       "$s" $m ''
				printf ' \e[1;33m%s\e[m' $w; echo
			}
		done
	done
}

run() {
	local -agr clang_s3=(c23 gnu23)     clang_v3=(18 19 20 21)
	local -agr clang_s2=(c2x gnu2x)     clang_v2=(9 1{0,1,2,3,4,5,6,7} "${clang_v3[@]}")
	local -agr clang_s1=(c18 gnu18)     clang_v1=(8 "${clang_v2[@]}")
	local -agr clang_s0=({c,gnu}1{1,7}) clang_v0=(6.0 7 "${clang_v1[@]}")
	local -agr gcc_s3=(c23 gnu23)       gcc_v3=(14 15)
	local -agr gcc_s2=(c2x gnu2x)       gcc_v2=(9 10 11 12 13 "${gcc_v3[@]}")
	local -agr gcc_s1=(c18 gnu18)       gcc_v1=(8 "${gcc_v2[@]}")
	local -agr gcc_s0=(c11 gnu11)       gcc_v0=(7 "${gcc_v1[@]}")
	local -ar sources=(src/args.c src/dbs26.c)
	local -ar c_flags=(-O3 -m{arch,tune}=native -W{all,extra,pedantic})
	local -i i
	for ((i = 0; i < 4; i++)); do
		run_ clang "$i" "${c_flags[@]}" -flto=full -fuse-ld=lld -Weverything  \
		           "${sources[@]}" "$@"
		run_ gcc   "$i" "${c_flags[@]}" -flto=auto \
		           "${sources[@]}" "$@"
	done

}

pushd "${0%/*}/" >/dev/null && {
	pushd .. >/dev/null && {
		run | sort -V -u
		popd >/dev/null
	}
	popd >/dev/null
}
