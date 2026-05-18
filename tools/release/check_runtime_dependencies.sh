#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

missing=0
required=(
	"bin/linux/godot_omt.linux.template_release.x86_64.so"
	"bin/linux/libomt.so"
	"bin/linux/libvmx.so"
	"bin/windows/godot_omt.windows.template_release.x86_64.dll"
	"bin/windows/libomt.dll"
	"bin/windows/libomt.lib"
	"bin/windows/libvmx.dll"
	"bin/windows/libomtnet.dll"
	"bin/macos/godot_omt.macos.template_release.universal.dylib"
	"bin/macos/libomt.dylib"
	"bin/macos/libvmx.dylib"
	"bin/macos/libomtnet.dll"
)

for file in "${required[@]}"; do
	if [[ -f "${ROOT}/${file}" ]]; then
		echo "ok: ${file}"
	else
		echo "missing: ${file}" >&2
		missing=1
	fi
done

if [[ "${missing}" != "0" ]]; then
	echo "One or more native runtime dependencies are missing." >&2
	exit 1
fi
