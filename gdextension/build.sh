#!/usr/bin/env bash
# Build godot_omt GDExtension for the parent Godot project.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${ROOT}/.." && pwd)"
BUILD_TYPE="${BUILD_TYPE:-debug}"
GODOT_BIN="${GODOT_BIN:-godot}"
REQUIRE_LIBOMT="${REQUIRE_LIBOMT:-1}"
BUILD_PLATFORM="${BUILD_PLATFORM:-}"

cd "${ROOT}"

LIBOMT_HEADER="${PROJECT_ROOT}/third_party/libomt/include/libomt.h"
if [[ ! -f "${LIBOMT_HEADER}" ]]; then
	echo "Fetching libomt.h..."
	mkdir -p "${PROJECT_ROOT}/third_party/libomt/include"
	curl -fsSL -o "${LIBOMT_HEADER}" \
		"https://raw.githubusercontent.com/openmediatransport/libomt/master/libomt.h"
fi

LIBOMT_ROOT="${LIBOMT_ROOT:-${PROJECT_ROOT}/third_party/libomt}"
HOST_PLATFORM="$(uname -s)"
if [[ -z "${BUILD_PLATFORM}" ]]; then
	case "${HOST_PLATFORM}" in
		Linux*) BUILD_PLATFORM="linux" ;;
		Darwin*) BUILD_PLATFORM="macos" ;;
		MINGW*|MSYS*|CYGWIN*) BUILD_PLATFORM="windows" ;;
		*) BUILD_PLATFORM="linux" ;;
	esac
fi

if [[ "${REQUIRE_LIBOMT}" == "1" && "${BUILD_PLATFORM}" == "linux" && ! -f "${LIBOMT_ROOT}/lib/libomt.so" ]]; then
	echo "libomt.so is missing; building/staging Linux libomt..."
	bash "${PROJECT_ROOT}/third_party/libomt/build-linux.sh"
fi

if [[ "${REQUIRE_LIBOMT}" == "1" ]]; then
	if [[ "${BUILD_PLATFORM}" == "linux" ]]; then
		if [[ ! -f "${LIBOMT_ROOT}/lib/libomt.so" ]]; then
			echo "error: ${LIBOMT_ROOT}/lib/libomt.so not found after build attempt." >&2
			echo "error: OMT discovery/receive cannot be enabled without libomt.so." >&2
			echo "error: rerun third_party/libomt/build-linux.sh and fix the first error it prints." >&2
			exit 1
		fi
		if [[ ! -f "${LIBOMT_ROOT}/lib/libvmx.so" ]]; then
			echo "error: ${LIBOMT_ROOT}/lib/libvmx.so not found after build attempt." >&2
			echo "error: libomt needs libvmx beside it at runtime." >&2
			exit 1
		fi
	fi
else
	if [[ "${BUILD_PLATFORM}" == "linux" && ! -f "${LIBOMT_ROOT}/lib/libomt.so" ]]; then
		echo "warning: ${LIBOMT_ROOT}/lib/libomt.so not found; OMT discovery/receive will be disabled." >&2
	fi
fi

if [[ ! -d godot-cpp/src ]]; then
	echo "Initializing godot-cpp submodule..."
	git submodule update --init --recursive godot-cpp
fi

if [[ ! -f godot-cpp/include/godot_cpp/godot.hpp ]]; then
	echo "Generating godot-cpp bindings (requires Godot 4.x on PATH as '${GODOT_BIN}')..."
	(cd godot-cpp && "${GODOT_BIN}" --headless --generate-extension-api godot_extension_api.json 2>/dev/null || true)
	(cd godot-cpp && python3 -m SCons platform=linux generate_bindings=yes 2>/dev/null || \
		cmake -S . -B build-bindings -DGODOTCPP_ENABLE_TESTING=OFF && cmake --build build-bindings --target generate_bindings 2>/dev/null || \
		echo "Run godot-cpp binding generation manually; see https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/build_system/index.html")
fi

CMAKE_BUILD_DIR="${ROOT}/build-${BUILD_TYPE}"
CMAKE_BUILD_TYPE="Debug"
if [[ "${BUILD_TYPE}" == "release" ]]; then
	CMAKE_BUILD_TYPE="Release"
fi

cmake -S "${ROOT}" -B "${CMAKE_BUILD_DIR}" \
	-DGODOTCPP_TARGET="template_${BUILD_TYPE}" \
	-DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
	-DLIBOMT_ROOT="${LIBOMT_ROOT}" \
	-DGODOT_OMT_REQUIRE_LIBOMT="${REQUIRE_LIBOMT}"

cmake --build "${CMAKE_BUILD_DIR}" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo "Built extension libraries into: ${PROJECT_ROOT}/bin/"
