# Godot OMT

Open Media Transport integration for Godot 4 via GDExtension.

This project is an early native Godot addon for receiving OMT video as Godot textures and sending Godot viewports as OMT sources. The current Linux path links against `libomt` and `libvmx`.

## Status

This is prototype-quality software. The clean loopback demo works on Linux with Godot 4.6 using Vulkan/Forward+, but the API surface and packaging are still expected to change.

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

## Repository Layout

```text
addons/godot_omt/          Godot editor addon and .gdextension manifest
doc_classes/              Godot class reference XML
gdextension/              C++ GDExtension source and build scripts
scenes/                   Demo scenes
scripts/                  Demo scripts
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
