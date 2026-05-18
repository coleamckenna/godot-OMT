#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

check_file() {
	local path="$1"
	if [[ -f "${path}" ]]; then
		printf 'OK      %s\n' "${path}"
	else
		printf 'MISSING %s\n' "${path}"
	fi
}

check_cmd() {
	local name="$1"
	if command -v "${name}" >/dev/null 2>&1; then
		printf 'OK      %s -> %s\n' "${name}" "$(command -v "${name}")"
	else
		printf 'MISSING command: %s\n' "${name}"
	fi
}

check_cmd git
if [[ -x "${HOME}/.dotnet/dotnet" ]]; then
	printf 'OK      dotnet -> %s\n' "${HOME}/.dotnet/dotnet"
else
	check_cmd dotnet
fi
check_cmd clang++
check_cmd patchelf

check_file "${ROOT}/include/libomt.h"
check_file "${ROOT}/lib/libomt.so"
check_file "${ROOT}/lib/libvmx.so"

printf '\nIf libomt/libvmx are missing, run:\n'
printf '  cd %s/../..\n' "${ROOT}"
printf '  bash third_party/libomt/build-linux.sh\n'
