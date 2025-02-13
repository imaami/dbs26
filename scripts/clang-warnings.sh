#!/usr/bin/env bash

run_() {
	local c o s s_="s$1[@]" v v_="v$1[@]" w
	local -i m n
	for s in "${!s_}"; do
		((m = ${#s}, m = m < 5 ? (5 - m) : 0))
		for v in "${!v_}"; do
			c="clang-$v"
			command -v "$c" >/dev/null || continue
			((n = ${#v}, n = n < 3 ? (3 - n) : 0))
			o=$("$c" "-std=$s" "${@:2}" -o "dbs26-$c-$s" 2>&1)
			w=$(grep -o -- '\[-W[^]][^]]*\]' <<< "$o"|sort -u)
			[[ -z "$w" ]] || {
				w="${w//[}"
				w="${w//]}"
				printf '\e[32m%s\e[m%*s \e[35m%s\e[m -std=\e[36m%s\e[m%*s' \
				       "$c" $n '' $("$c" "-std=$s" -xc -E -dM - <<< ''     \
				                   | grep __STDC_VERSION__ | cut -d_ -f6-) \
				       "$s" $m ''
				printf ' \e[1;33m%s\e[m' $w; echo
			}
		done
	done
}

run() {
	local -agr s3=(c23 gnu23)     v3=(18 19 20 21)
	local -agr s2=(c2x gnu2x)     v2=(9 1{0,1,2,3,4,5,6,7} "${v3[@]}")
	local -agr s1=(c18 gnu18)     v1=(8 "${v2[@]}")
	local -agr s0=({c,gnu}1{1,7}) v0=(6.0 7 "${v1[@]}")
	local -i i
	for ((i = 0; i < 4; i++)); do
		run_ "$i" -{O3,m{arch,tune}=znver1,flto=full,fuse-ld=lld} \
		     -W{all,extra,pedantic,everything} src/{dbs26,args}.c \
		     "$@"
	done
}

pushd "${0%/*}/" >/dev/null && {
	pushd .. >/dev/null && {
		run | sort -V -u
		popd >/dev/null
	}
	popd >/dev/null
}
