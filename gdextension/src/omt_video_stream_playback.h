#pragma once

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/video_stream_playback.hpp>

namespace godot {

class OMTVideoStreamPlayback : public VideoStreamPlayback {
	GDCLASS(OMTVideoStreamPlayback, VideoStreamPlayback);

private:
	String source_address;
	bool preview_mode = false;
	int preferred_format = 2;
	bool playing = false;
	bool paused = false;
	double playback_position = 0.0;
	Ref<ImageTexture> texture;

protected:
	static void _bind_methods();

public:
	OMTVideoStreamPlayback();

	void configure(const String &p_source_address, bool p_preview_mode, int p_preferred_format);

	void _play() override;
	void _stop() override;
	bool _is_playing() const override;
	void _set_paused(bool p_paused) override;
	bool _is_paused() const override;
	double _get_length() const override;
	double _get_playback_position() const override;
	void _seek(double p_time) override;
	void _set_audio_track(int32_t p_idx) override;
	Ref<Texture2D> _get_texture() const override;
	void _update(double p_delta) override;
	int32_t _get_channels() const override;
	int32_t _get_mix_rate() const override;
};

} // namespace godot
