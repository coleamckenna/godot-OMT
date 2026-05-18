#pragma once

#include <godot_cpp/classes/video_stream.hpp>

namespace godot {

class OMTVideoStream : public VideoStream {
	GDCLASS(OMTVideoStream, VideoStream);

private:
	String source_address;
	bool preview_mode = false;
	int preferred_format = 2;

protected:
	static void _bind_methods();

public:
	Ref<VideoStreamPlayback> _instantiate_playback() override;

	void set_source_address(const String &p_address);
	String get_source_address() const;
	void set_preview_mode(bool p_enabled);
	bool get_preview_mode() const;
	void set_preferred_format(int p_format);
	int get_preferred_format() const;
};

} // namespace godot
