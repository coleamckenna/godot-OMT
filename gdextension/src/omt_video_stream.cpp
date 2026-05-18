#include "omt_video_stream.h"

#include "omt_video_stream_playback.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void OMTVideoStream::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_source_address", "address"), &OMTVideoStream::set_source_address);
	ClassDB::bind_method(D_METHOD("get_source_address"), &OMTVideoStream::get_source_address);
	ClassDB::bind_method(D_METHOD("set_preview_mode", "enabled"), &OMTVideoStream::set_preview_mode);
	ClassDB::bind_method(D_METHOD("get_preview_mode"), &OMTVideoStream::get_preview_mode);
	ClassDB::bind_method(D_METHOD("set_preferred_format", "format"), &OMTVideoStream::set_preferred_format);
	ClassDB::bind_method(D_METHOD("get_preferred_format"), &OMTVideoStream::get_preferred_format);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "source_address"), "set_source_address", "get_source_address");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "preview_mode"), "set_preview_mode", "get_preview_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "preferred_format", PROPERTY_HINT_ENUM, "UYVY,UYVY or BGRA,BGRA,UYVY or UYVA,UYVY or UYVA or P216 or PA16,P216"), "set_preferred_format", "get_preferred_format");
}

Ref<VideoStreamPlayback> OMTVideoStream::_instantiate_playback() {
	Ref<OMTVideoStreamPlayback> playback;
	playback.instantiate();
	playback->configure(source_address, preview_mode, preferred_format);
	return playback;
}

void OMTVideoStream::set_source_address(const String &p_address) {
	source_address = p_address;
}

String OMTVideoStream::get_source_address() const {
	return source_address;
}

void OMTVideoStream::set_preview_mode(bool p_enabled) {
	preview_mode = p_enabled;
}

bool OMTVideoStream::get_preview_mode() const {
	return preview_mode;
}

void OMTVideoStream::set_preferred_format(int p_format) {
	preferred_format = p_format;
}

int OMTVideoStream::get_preferred_format() const {
	return preferred_format;
}
