# OMT Utility Release Checklist

These utilities are exported from the main Godot OMT project:

- `OMT Monitor`: `res://tools/monitor/omt_monitor.tscn`
  Discovers OMT sources, receives full-quality video by default, displays audio/video/metadata stats, and offers stream-only fullscreen inside the app window.
- `OMT Camera Mic`: `res://tools/camera_mic_sender/omt_camera_mic_sender.tscn`
  Publishes a selected Godot `CameraServer` feed plus microphone input as an OMT source. The selected `CameraTexture` is activated before use and output quality defaults to 90.
- `OMT Test Pattern`: `res://tools/test_pattern_generator/omt_test_pattern_generator.tscn`
  Publishes generated visual patterns, metadata, and a sine test tone. The tone is sent over OMT and played locally with `AudioStreamGenerator`.

## Build

1. Build the native GDExtension and OMT runtime libraries for each target OS.
2. Stage Windows and macOS OMT runtime libraries:

```bash
tools/release/stage_omt_desktop_runtimes.sh
```

3. Run `tools/release/check_runtime_dependencies.sh`.
4. Run `tools/release/export_all.sh` with `GODOT_BIN` set if Godot is not on `PATH`.

`export_all.sh` temporarily switches `project.godot` to each utility's main
scene while exporting, then restores the project back to the normal demo menu.
This is required because Godot exports otherwise inherit the project-level main
scene for every preset.

The Windows and macOS GDExtension binaries must be built on matching hosted
runners or native machines. The `Build GDExtension` GitHub Actions workflow
uploads platform `bin/` folders as artifacts.

## Validation

1. Launch `OMT Test Pattern`, start publishing, and confirm it appears in `OMT Monitor`.
2. Confirm the test tone is audible locally, then connect `OMT Monitor` and confirm audio frame stats update.
3. Change test pattern, resolution, frame rate, metadata, and tone frequency while running.
4. Use `OMT Monitor` stream fullscreen and confirm only the video area expands, not the whole OS window.
5. Launch `OMT Camera Mic`, select a camera and microphone, start publishing, and confirm live video plus audio metadata in `OMT Monitor`.
6. Stop each sender and confirm the monitor source list refreshes cleanly.
7. Temporarily remove a native runtime library and confirm the app reports the missing OMT runtime instead of failing silently.

## Android

Android is not part of the release set yet. The current upstream OMT binary
package includes Windows and macOS native libraries, but does not include
Android `libomt`/`libvmx` ARM64 libraries. A stub Android export could be added
with `GODOT_OMT_REQUIRE_LIBOMT=OFF`, but a real OMT Android utility release
should wait until Android ARM64 OMT runtime libraries are available and tested.
