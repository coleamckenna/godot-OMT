# Godot OMT

Open Media Transport integration for Godot 4 via GDExtension.

This project is an early native Godot addon for receiving OMT video as Godot textures and sending Godot viewports as OMT sources. The current Linux path links against `libomt` and `libvmx`.

## Status

This is prototype-quality software. The clean loopback demo works on Linux with Godot 4.6 using Vulkan/Forward+, but the API surface and packaging are still expected to change.

Apologies up front: this repository is currently AI slop in the literal sense that a lot of the demo and utility scaffolding was assembled quickly with AI assistance. It is useful for exploration and smoke testing, but it still needs real review, cleanup, device testing, and platform-specific hardening before it should be treated as production software.

## Features

- `OMTReceiver` receives OMT video frames into a `Texture2D`.
- `OMTOutput` sends a `Viewport` or `SubViewport` as an OMT source.
- `OMTDiscovery` exposes refreshable OMT source discovery.
- `OMT` exposes runtime status helpers for checking whether native OMT support is available.
- `OMTVideoStream` and `OMTVideoStreamPlayback` provide the start of a `VideoStreamPlayer` integration.
- `addons/godot_omt` adds editor tooling, including a refreshable source picker for receiver source addresses.

## Demos

The default scene is now a demo menu:

```text
res://scenes/omt_demo_menu.tscn
```

From there you can open focused scenes for:

- `OMT` runtime status helpers.
- `OMTDiscovery` source refreshes and source change signals.
- `OMTReceiver` texture display using test-pattern mode.
- `OMTOutput` publishing an animated `SubViewport`.
- `OMTOutput -> libomt -> OMTReceiver` clean loopback.
- `OMTVideoStream` assigned to a `VideoStreamPlayer`.

The clean loopback scene remains the best smoke test after building the extension.

### Demo Purposes

- `Runtime Status` shows the native OMT runtime availability helpers and any load errors.
- `Discovery` exercises `OMTDiscovery` refreshes, source-added/source-removed signals, and source list rendering.
- `Receiver Texture` shows an `OMTReceiver` texture in Godot using test-pattern mode.
- `Output` publishes an animated `SubViewport` through `OMTOutput`.
- `Clean Loopback` sends a local `SubViewport` through `OMTOutput` and receives it back through `OMTReceiver`.
- `Video Stream` demonstrates the early `OMTVideoStream` / `VideoStreamPlayer` integration path.

## Standalone Utilities

The project also contains three utility scenes intended to be exported as small desktop apps:

- `OMT Monitor` (`res://tools/monitor/omt_monitor.tscn`) discovers OMT sources, connects to a selected source, displays the received texture, and shows video, audio, metadata, and sender statistics. It uses full-quality receiver settings by default and its fullscreen button only expands the OMT stream inside the app UI instead of switching the whole OS window to fullscreen.
- `OMT Camera Mic` (`res://tools/camera_mic_sender/omt_camera_mic_sender.tscn`) publishes a selected Godot `CameraServer` feed and microphone input as an OMT source. It activates the selected `CameraTexture`, renders it through a `SubViewport`, captures microphone samples with `AudioEffectCapture`, and defaults to higher output quality.
- `OMT Test Pattern` (`res://tools/test_pattern_generator/omt_test_pattern_generator.tscn`) publishes generated SMPTE-style bars, checkerboard, or grid patterns with metadata and a sine-wave test tone. The tone is sent over OMT and also played locally through Godot's `AudioStreamGenerator` so you can confirm audio output without needing a separate receiver.

Export presets for Linux, Windows, and macOS live in `export_presets.cfg`. Linux exports can be produced locally once the Linux GDExtension and OMT runtime libraries are staged. Windows and macOS exports require matching native GDExtension binaries in `bin/windows` and `bin/macos`; the GitHub Actions build workflow is the intended path for those artifacts.

## Repository Layout

```text
addons/godot_omt/          Godot editor addon and .gdextension manifest
doc_classes/              Godot class reference XML
gdextension/              C++ GDExtension source and build scripts
scenes/                   Demo scenes
scripts/                  Demo scripts
tools/                    Standalone OMT utilities and release scripts
third_party/libomt/        libomt staging/build helper scripts
```

Generated Godot metadata, native binaries, CMake build folders, staged `.so` files, and other build outputs are intentionally ignored.

## Dependencies

- Godot 4.6 or newer is recommended for the current demo.
- CMake and a C++17-capable compiler.
- `godot-cpp`, checked out as a git submodule at `gdextension/godot-cpp`.
- `libomt.so` and `libvmx.so`, staged under `third_party/libomt/lib/`.
- .NET 8 SDK is required when building `libomt` from source.

## Build On Linux

Clone with submodules:

```bash
git clone --recurse-submodules <repo-url>
cd godot-OMT
```

If you already cloned without submodules:

```bash
git submodule update --init --recursive
```

Check prerequisites and staged native libraries:

```bash
bash third_party/libomt/check-status.sh
```

Build and stage OMT libraries:

```bash
bash third_party/libomt/build-linux.sh
```

The helper clones/builds upstream OMT dependencies under `.build/omt` by default. Set `OMT_WORK_ROOT=/path/to/existing/worktree` if you want to reuse existing `libomtnet`, `libomt`, and `libvmx` checkouts instead.

Build the GDExtension:

```bash
bash gdextension/build.sh
BUILD_TYPE=release bash gdextension/build.sh
```

Expected runtime layout after building:

```text
bin/linux/godot_omt.linux.template_debug.x86_64.so
bin/linux/godot_omt.linux.template_release.x86_64.so
bin/linux/libomt.so
bin/linux/libvmx.so
```

## Usage

Open the project in Godot and run the default loopback scene. For a custom receiver:

1. Add an `OMTReceiver` node.
2. Set `source_address` to a discovered OMT source.
3. Display `receiver.get_texture()` in a `TextureRect`, `Sprite2D`, or material.

For a custom output:

1. Add an `OMTOutput` node.
2. Set `viewport_path` to the `Viewport` or `SubViewport` you want to send.
3. Enable the node or call `start()`.

## License

This addon is licensed under the MIT License. See `LICENSE`.

Third-party projects keep their own licenses. In particular, `godot-cpp`, `libomt`, and `libvmx` are separate upstream dependencies.
