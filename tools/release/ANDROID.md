# Android Export Status

Android exports are not enabled for the OMT utilities yet.

## Current Decision

Do not add Android utility presets for the release path until native OMT Android
runtime libraries are available. A stub Android build is possible with
`GODOT_OMT_REQUIRE_LIBOMT=OFF`, but it would not provide real OMT discovery,
receive, or send behavior.

## What Is Missing

- Android ARM64 `libomt.so`.
- Android ARM64 `libvmx.so`.
- A matching Android `godot_omt` GDExtension build.
- Android entries in `addons/godot_omt/godot_omt.gdextension`.
- Android export presets and runtime permissions.
- Device testing for LAN discovery, camera input, microphone input, and audio
  route behavior.

## Recommended Path

1. Obtain or build Android ARM64 OMT runtime libraries.
2. Add `android.debug.arm64` and `android.release.arm64` GDExtension entries.
3. Stage Android libraries under the Godot Android native library layout.
4. Add Android export presets for the utilities.
5. Validate on a physical ARM64 Android device.
