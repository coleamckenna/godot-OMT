#include "omt_frame_converter.h"

#include <algorithm>
#include <cstring>

namespace godot_omt {

void bgra_to_rgba(const uint8_t *src, uint8_t *dst, size_t pixel_count) {
	for (size_t i = 0; i < pixel_count; ++i) {
		const size_t o = i * 4;
		dst[o + 0] = src[o + 2];
		dst[o + 1] = src[o + 1];
		dst[o + 2] = src[o + 0];
		dst[o + 3] = src[o + 3];
	}
}

void uyvy_to_rgba(
		const uint8_t *src,
		int width,
		int height,
		int stride_bytes,
		std::vector<uint8_t> &out_rgba) {
	const size_t out_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
	out_rgba.resize(out_size);

	for (int y = 0; y < height; ++y) {
		const uint8_t *row = src + static_cast<size_t>(y) * static_cast<size_t>(stride_bytes);
		for (int x = 0; x < width; x += 2) {
			const int pair_index = x / 2;
			const uint8_t u = row[pair_index * 4 + 0];
			const uint8_t y0 = row[pair_index * 4 + 1];
			const uint8_t v = row[pair_index * 4 + 2];
			const uint8_t y1 = row[pair_index * 4 + 3];

			const auto yuv_to_rgb = [](int y_val, int u_val, int v_val, uint8_t &r, uint8_t &g, uint8_t &b) {
				const int c = y_val - 16;
				const int d = u_val - 128;
				const int e = v_val - 128;
				int r_i = (298 * c + 409 * e + 128) >> 8;
				int g_i = (298 * c - 100 * d - 208 * e + 128) >> 8;
				int b_i = (298 * c + 516 * d + 128) >> 8;
				r = static_cast<uint8_t>(std::clamp(r_i, 0, 255));
				g = static_cast<uint8_t>(std::clamp(g_i, 0, 255));
				b = static_cast<uint8_t>(std::clamp(b_i, 0, 255));
			};

			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;
			yuv_to_rgb(y0, u, v, r, g, b);
			const size_t o0 = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 4;
			out_rgba[o0 + 0] = r;
			out_rgba[o0 + 1] = g;
			out_rgba[o0 + 2] = b;
			out_rgba[o0 + 3] = 255;

			if (x + 1 < width) {
				yuv_to_rgb(y1, u, v, r, g, b);
				const size_t o1 = o0 + 4;
				out_rgba[o1 + 0] = r;
				out_rgba[o1 + 1] = g;
				out_rgba[o1 + 2] = b;
				out_rgba[o1 + 3] = 255;
			}
		}
	}
}

} // namespace godot_omt
