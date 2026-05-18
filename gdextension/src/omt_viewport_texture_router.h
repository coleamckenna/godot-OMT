#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class RDTextureFormat;

class OMTViewportTextureRouter : public Object {
	GDCLASS(OMTViewportTextureRouter, Object);

private:
	Dictionary viewports;
	int64_t frames_observed = 0;
	int64_t readback_requests = 0;
	int64_t readback_callbacks = 0;
	String last_error;

	void _request_textures();
	void _forward_texture(PackedByteArray p_data, const Ref<RDTextureFormat> &p_format, int64_t p_viewport_rid);
	void _connect_frame_signal();
	void _disconnect_frame_signal();
	void _set_last_error(const String &p_error);

protected:
	static void _bind_methods();

public:
	OMTViewportTextureRouter();
	~OMTViewportTextureRouter() override;

	void add_viewport(Viewport *p_viewport);
	void remove_viewport(Viewport *p_viewport);
	Dictionary get_status() const;
};

} // namespace godot
