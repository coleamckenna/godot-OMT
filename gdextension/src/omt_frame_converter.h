#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace godot_omt {

// Godot Image FORMAT_RGBA8 is R,G,B,A order.
void bgra_to_rgba(const uint8_t *src, uint8_t *dst, size_t pixel_count);

// Expand UYVY 4:2:2 packed rows to RGBA8 (CPU path for phase one).
void uyvy_to_rgba(
		const uint8_t *src,
		int width,
		int height,
		int stride_bytes,
		std::vector<uint8_t> &out_rgba);

} // namespace godot_omt
