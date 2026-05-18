#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
GODOT_BIN="${GODOT_BIN:-godot}"
PROJECT_FILE="${ROOT}/project.godot"
PROJECT_BACKUP="$(mktemp)"

restore_project_file() {
	cp "${PROJECT_BACKUP}" "${PROJECT_FILE}"
	rm -f "${PROJECT_BACKUP}"
}

set_main_scene() {
	local scene="$1"
	python3 - "${PROJECT_FILE}" "${scene}" <<'PY'
from pathlib import Path
import sys

project_file = Path(sys.argv[1])
scene = sys.argv[2]
text = project_file.read_text()
next_text = []
replaced = False

for line in text.splitlines():
    if line.startswith('run/main_scene='):
        next_text.append(f'run/main_scene="{scene}"')
        replaced = True
    else:
        next_text.append(line)

if not replaced:
    raise SystemExit("project.godot does not contain run/main_scene")

project_file.write_text("\n".join(next_text) + "\n")
PY
}

cd "${ROOT}"
mkdir -p builds/linux builds/windows builds/macos
cp "${PROJECT_FILE}" "${PROJECT_BACKUP}"
trap restore_project_file EXIT

presets=(
	"OMT Monitor Linux|builds/linux/omt-monitor.x86_64|res://tools/monitor/omt_monitor.tscn"
	"OMT Camera Mic Linux|builds/linux/omt-camera-mic.x86_64|res://tools/camera_mic_sender/omt_camera_mic_sender.tscn"
	"OMT Test Pattern Linux|builds/linux/omt-test-pattern.x86_64|res://tools/test_pattern_generator/omt_test_pattern_generator.tscn"
	"OMT Monitor Windows|builds/windows/OMT Monitor.exe|res://tools/monitor/omt_monitor.tscn"
	"OMT Camera Mic Windows|builds/windows/OMT Camera Mic.exe|res://tools/camera_mic_sender/omt_camera_mic_sender.tscn"
	"OMT Test Pattern Windows|builds/windows/OMT Test Pattern.exe|res://tools/test_pattern_generator/omt_test_pattern_generator.tscn"
	"OMT Monitor macOS|builds/macos/OMT Monitor.zip|res://tools/monitor/omt_monitor.tscn"
	"OMT Camera Mic macOS|builds/macos/OMT Camera Mic.zip|res://tools/camera_mic_sender/omt_camera_mic_sender.tscn"
	"OMT Test Pattern macOS|builds/macos/OMT Test Pattern.zip|res://tools/test_pattern_generator/omt_test_pattern_generator.tscn"
)

for entry in "${presets[@]}"; do
	IFS="|" read -r preset output_path main_scene <<< "${entry}"
	if [[ -n "${EXPORT_PRESET_FILTER:-}" && "${preset}" != *"${EXPORT_PRESET_FILTER}"* ]]; then
		continue
	fi
	echo "Exporting ${preset} -> ${output_path} (${main_scene})..."
	set_main_scene "${main_scene}"
	"${GODOT_BIN}" --headless --export-release "${preset}" "${output_path}"
done
