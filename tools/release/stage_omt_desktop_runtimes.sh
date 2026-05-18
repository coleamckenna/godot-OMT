#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VERSION="${OMT_BINARIES_VERSION:-v1.0.0.14}"
URL="${OMT_BINARIES_URL:-https://github.com/openmediatransport/libomtnet/releases/download/${VERSION}/OpenMediaTransport.Binaries.Release.${VERSION}.zip}"
WORK_DIR="${ROOT}/.build/omt-binaries"
ZIP_PATH="${WORK_DIR}/OpenMediaTransport.Binaries.Release.${VERSION}.zip"
EXTRACT_DIR="${WORK_DIR}/extracted"

mkdir -p "${WORK_DIR}" "${ROOT}/bin/windows" "${ROOT}/bin/macos"

if [[ ! -f "${ZIP_PATH}" ]]; then
	echo "Downloading ${URL}"
	curl -L -o "${ZIP_PATH}" "${URL}"
fi

rm -rf "${EXTRACT_DIR}"
unzip -q "${ZIP_PATH}" -d "${EXTRACT_DIR}"

cp -f \
	"${EXTRACT_DIR}/Libraries/Winx64/libomt.dll" \
	"${EXTRACT_DIR}/Libraries/Winx64/libomt.lib" \
	"${EXTRACT_DIR}/Libraries/Winx64/libvmx.dll" \
	"${EXTRACT_DIR}/Libraries/Winx64/libomtnet.dll" \
	"${ROOT}/bin/windows/"

cp -f \
	"${EXTRACT_DIR}/Libraries/MacOS/libomt.dylib" \
	"${EXTRACT_DIR}/Libraries/MacOS/libvmx.dylib" \
	"${EXTRACT_DIR}/Libraries/MacOS/libomtnet.dll" \
	"${ROOT}/bin/macos/"

echo "Staged Windows OMT runtime files in ${ROOT}/bin/windows"
echo "Staged macOS OMT runtime files in ${ROOT}/bin/macos"
