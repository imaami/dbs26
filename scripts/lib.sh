(return 0 2>/dev/null)              &&
[[ -z ${DBS26_SCRIPTS_LIB_SH_+1} ]] &&
declare -g DBS26_SCRIPTS_LIB_SH_=1  && {
########################################

get_compilers() {
	local -a arr=($({ compgen -c clang; compgen -c gcc; } |
		grep -E '^(clang|gcc)(-[0-9]+(\.[0-9]+)*)?$'  |
		sort -V -u)
	)
	local p x=" ${!arr[@]}"
	printf -vx -- "${x// / [\"%s\"]=}" "${arr[@]}"
	eval "local -Ai index_by_name=(${x:1})"
	local -A name_by_path
	for x in ${arr[@]%%*[^0-9]} ${arr[@]//*-*}; do
		p=$(realpath "$(command -v "$x" 2>/dev/null)")
		[[ -z "$p" ]] || name_by_path["$p"]="$x"
	done
	unset arr
	local -a arr
	for x in "${name_by_path[@]}"; do
		local -i i=${index_by_name["$x"]}
		arr[i]="$x"
	done
	declare -ag compilers=("${arr[@]}")
}

##########
} || ((1))
