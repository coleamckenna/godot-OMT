#!/usr/bin/env bash
# Point this at a libomt build or extracted OpenMediaTransport binaries.
# Prebuilt zip (Windows/macOS): https://github.com/openmediatransport/libomtnet/releases
#
# Linux: build libomt from https://github.com/openmediatransport/libomt
# (requires libomtnet sibling + .NET 8 for Native AOT), then:
#   export LIBOMT_INSTALL=/path/to/libomt/output
#   ./third_party/libomt/setup-libs.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL="${LIBOMT_INSTALL:-}"

if [[ -z "${INSTALL}" || ! -d "${INSTALL}" ]]; then
	echo "Set LIBOMT_INSTALL to a directory containing libomt.so and libvmx.so"
	exit 1
fi

mkdir -p "${ROOT}/lib" "${ROOT}/include"
cp -a "${INSTALL}/"* "${ROOT}/lib/" 2>/dev/null || true
find "${INSTALL}" -maxdepth 3 \( -name 'libomt.so' -o -name 'libomt.dylib' -o -name 'libomt.dll' -o -name 'libvmx.so' -o -name 'libvmx.dylib' -o -name 'libvmx.dll' \) -exec cp -a {} "${ROOT}/lib/" \;
cp -f "${INSTALL}/libomt.h" "${ROOT}/include/libomt.h" 2>/dev/null || true

if command -v patchelf >/dev/null 2>&1 && [[ -f "${ROOT}/lib/libomt.so" ]]; then
	patchelf --set-rpath '$ORIGIN' "${ROOT}/lib/libomt.so" || true
fi

echo "Libraries staged under ${ROOT}/lib — rebuild with: gdextension/build.sh"
