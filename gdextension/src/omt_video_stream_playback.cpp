#include "omt_video_stream_playback.h"

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

OMTVideoStreamPlayback::OMTVideoStreamPlayback() {
	texture.instantiate();
	Ref<Image> image = Image::create(2, 2, false, Image::FORMAT_RGBA8);
	texture->set_image(image);
}

void OMTVideoStreamPlayback::_bind_methods() {
	ClassDB::bind_method(D_METHOD("configure", "source_address", "preview_mode", "preferred_format"), &OMTVideoStreamPlayback::configure);
}

void OMTVideoStreamPlayback::configure(const String &p_source_address, bool p_preview_mode, int p_preferred_format) {
	source_address = p_source_address;
	preview_mode = p_preview_mode;
	preferred_format = p_preferred_format;
}

void OMTVideoStreamPlayback::_play() {
	playing = true;
	paused = false;
}

void OMTVideoStreamPlayback::_stop() {
	playing = false;
	paused = false;
	playback_position = 0.0;
}

bool OMTVideoStreamPlayback::_is_playing() const {
	return playing;
}

void OMTVideoStreamPlayback::_set_paused(bool p_paused) {
	paused = p_paused;
}

bool OMTVideoStreamPlayback::_is_paused() const {
	return paused;
}

double OMTVideoStreamPlayback::_get_length() const {
	return 0.0;
}

double OMTVideoStreamPlayback::_get_playback_position() const {
	return playback_position;
}

void OMTVideoStreamPlayback::_seek(double p_time) {
	playback_position = p_time;
}

void OMTVideoStreamPlayback::_set_audio_track(int32_t p_idx) {
	(void)p_idx;
}

Ref<Texture2D> OMTVideoStreamPlayback::_get_texture() const {
	return texture;
}

void OMTVideoStreamPlayback::_update(double p_delta) {
	if (playing && !paused) {
		playback_position += p_delta;
	}
}

int32_t OMTVideoStreamPlayback::_get_channels() const {
	return 0;
}

int32_t OMTVideoStreamPlayback::_get_mix_rate() const {
	return 0;
}
