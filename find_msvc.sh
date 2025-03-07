#!/usr/bin/env bash

find_msvc()
{
	local msvc_dir="$1" build_arch="$2" host_arch="$3"
	local msvc_ver=$(dos2unix < "$msvc_dir/VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt")
	local sdk_ver=$(dos2unix < "$msvc_dir/Windows Kits/10/SDKManifest.xml" \
	| grep -Eo 'PlatformIdentity *= *"[^"]+"' | grep -Eo '[0-9]+(\.[0-9]+)+')
	local base="Z:${msvc_dir//\//\\}"
	local vc="$base\\VC\\Tools\\MSVC\\$msvc_ver"
	local sdk="$base\\Windows Kits\\10"
	local inc="$sdk\\Include\\$sdk_ver"
	local lib="$sdk\\Lib\\$sdk_ver"

	local -gx INCLUDE="$vc\\include;$inc\\ucrt;$inc\\um;$inc\\shared;$inc\\winrt"
	local -gx LIB="$vc\\lib\\$host_arch;$lib\\ucrt\\$host_arch;$lib\\um\\$host_arch"
	local -gx WINEPATH="$vc\\bin\\Host$build_arch\\$host_arch"
	WINEPATH+=";$sdk\\bin\\$sdk_ver\\$host_arch"
	set -- $(command -v wine64 wine-stable wine 2>/dev/null || true)
	printf '%s\n' '[constants]'         \
	       "msvc_sdk = '$msvc_dir'"     \
	       "msvc_version = '$msvc_ver'" \
	       "wine_exe = [ '${1##*/}' ]"  \
	       "llvm_suffix = ''"
}

find_msvc "$HOME/.local/msvc" arm64 arm64
