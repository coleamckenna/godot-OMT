#!/usr/bin/env bash
# Build and stage Open Media Transport C API libraries for Linux.
#
# Output:
#   third_party/libomt/include/libomt.h
#   third_party/libomt/lib/libomt.so
#   third_party/libomt/lib/libvmx.so
#
# This expects the upstream repo layout:
#   <WORK_ROOT>/libomtnet
#   <WORK_ROOT>/libomt
#   <WORK_ROOT>/libvmx
#
# By default it reuses /home/cole/omt-obs-linux if present because that tree
# already has libomtnet/libvmx checkouts from the OBS OMT Linux work.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
WORK_ROOT="${OMT_WORK_ROOT:-/home/cole/omt-obs-linux}"

if [[ ! -d "${WORK_ROOT}" ]]; then
	WORK_ROOT="${OMT_WORK_ROOT:-${PROJECT_ROOT}/.build/omt}"
fi

log() {
	printf '==> %s\n' "$*"
}

need_cmd() {
	if ! command -v "$1" >/dev/null 2>&1; then
		printf 'error: required command not found: %s\n' "$1" >&2
		printf 'install it, then rerun: BUILD_LIBOMT=1 bash gdextension/build.sh\n' >&2
		exit 1
	fi
}

ensure_dotnet() {
	if [[ -x "${HOME}/.dotnet/dotnet" ]]; then
		export DOTNET_ROOT="${HOME}/.dotnet"
		export PATH="${HOME}/.dotnet:${PATH}"
	fi

	if command -v dotnet >/dev/null 2>&1; then
		return
	fi

	if ! command -v curl >/dev/null 2>&1; then
		printf 'error: dotnet is missing and curl is required to install .NET 8 locally\n' >&2
		printf 'install curl or install .NET 8 SDK manually, then rerun this script\n' >&2
		exit 1
	fi

	log "Installing .NET 8 SDK to ${HOME}/.dotnet"
	mkdir -p "${HOME}/.dotnet"
	curl -fsSL https://dot.net/v1/dotnet-install.sh -o /tmp/dotnet-install.sh
	bash /tmp/dotnet-install.sh --channel 8.0 --install-dir "${HOME}/.dotnet"
	export DOTNET_ROOT="${HOME}/.dotnet"
	export PATH="${HOME}/.dotnet:${PATH}"

	need_cmd dotnet
}

clone_if_missing() {
	local name="$1"
	if [[ ! -d "${WORK_ROOT}/${name}" ]]; then
		log "Cloning ${name}"
		git clone --depth 1 "https://github.com/openmediatransport/${name}.git" "${WORK_ROOT}/${name}"
	fi
}

find_native_output() {
	local root="$1"
	local name="$2"
	local match
	match="$(find "${root}" -path '*/bin/Release/net8.0/linux-x64/native/*' -name "${name}" -print -quit)"
	if [[ -z "${match}" ]]; then
		match="$(find "${root}" -name "${name}" -print -quit)"
	fi
	printf '%s' "${match}"
}

main() {
	need_cmd git
	ensure_dotnet

	if ! command -v clang++ >/dev/null 2>&1; then
		printf 'error: clang++ is required to build libvmx.so\n' >&2
		printf 'on Ubuntu/Debian: sudo apt install clang\n' >&2
		exit 1
	fi

	mkdir -p "${WORK_ROOT}" "${SCRIPT_DIR}/include" "${SCRIPT_DIR}/lib"

	clone_if_missing libomtnet
	clone_if_missing libomt
	clone_if_missing libvmx

	log "Building libvmx.so"
	(
		cd "${WORK_ROOT}/libvmx/build"
		chmod +x buildlinuxx64.sh
		./buildlinuxx64.sh
	)

	log "Building libomtnet"
	(
		cd "${WORK_ROOT}/libomtnet/build"
		chmod +x buildall.sh
		./buildall.sh
	)

	log "Publishing libomt.so"
	(
		cd "${WORK_ROOT}/libomt/build"
		chmod +x buildlinuxx64.sh
		./buildlinuxx64.sh
	)

	libomt_so="$(find_native_output "${WORK_ROOT}/libomt" 'libomt.so')"
	libvmx_so="${WORK_ROOT}/libvmx/build/libvmx.so"

	if [[ ! -f "${libomt_so}" ]]; then
		printf 'error: libomt.so was not produced by the libomt build\n' >&2
		printf 'checked under: %s\n' "${WORK_ROOT}/libomt" >&2
		printf 'try manually: cd %s/libomt/build && ./buildlinuxx64.sh\n' "${WORK_ROOT}" >&2
		exit 1
	fi

	if [[ ! -f "${libvmx_so}" ]]; then
		printf 'error: libvmx.so was not produced by the libvmx build\n' >&2
		printf 'checked: %s\n' "${libvmx_so}" >&2
		printf 'try manually: cd %s/libvmx/build && ./buildlinuxx64.sh\n' "${WORK_ROOT}" >&2
		exit 1
	fi

	cp -f "${WORK_ROOT}/libomt/libomt.h" "${SCRIPT_DIR}/include/libomt.h"
	cp -f "${libomt_so}" "${SCRIPT_DIR}/lib/libomt.so"
	cp -f "${libvmx_so}" "${SCRIPT_DIR}/lib/libvmx.so"

	if command -v patchelf >/dev/null 2>&1; then
		patchelf --set-rpath '$ORIGIN' "${SCRIPT_DIR}/lib/libomt.so" || true
	fi

	log "Staged ${SCRIPT_DIR}/lib/libomt.so"
	log "Staged ${SCRIPT_DIR}/lib/libvmx.so"
	log "Now rebuild the GDExtension: cd ${PROJECT_ROOT}/gdextension && ./build.sh"
}

main "$@"
