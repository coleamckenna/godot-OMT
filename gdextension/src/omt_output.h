#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>
#include <vector>

namespace godot {

class RDTextureFormat;

class OMTOutput : public Node {
	GDCLASS(OMTOutput, Node);

private:
	String source_name = "Godot OMT Output";
	NodePath viewport_path;
	bool enabled = false;
	int quality = 50;
	int frame_rate = 60;
	bool include_alpha = false;
	String metadata;
	double frame_accumulator = 0.0;
	String last_error;
	int64_t frames_sent = 0;
	int64_t frames_attempted = 0;
	int last_send_result = 0;
	int64_t last_frame_checksum = 0;
	int64_t readback_callbacks = 0;
	std::vector<uint8_t> send_bgra;
	Viewport *registered_viewport = nullptr;

#ifndef GODOT_OMT_NO_LIBOMT
	void *omt_send_handle = nullptr;
#endif

	void _create_sender();
	void _destroy_sender();
	void _register_viewport_router();
	void _unregister_viewport_router();
	Viewport *_get_target_viewport();
	void _send_texture(PackedByteArray p_data, const Ref<RDTextureFormat> &p_format, int64_t p_viewport_rid);
	void _send_metadata_frame(const String &p_metadata);
	static void _rgba_to_bgra(const uint8_t *p_src, uint8_t *p_dst, size_t p_pixel_count);

protected:
	static void _bind_methods();

public:
	OMTOutput();
	~OMTOutput() override;

	void _ready() override;
	void _process(double p_delta) override;
	void _exit_tree() override;

	void start();
	void stop();
	bool is_running() const;
	String get_address() const;
	String get_last_error() const;
	int64_t get_frames_sent() const;
	int64_t get_frames_attempted() const;
	int get_last_send_result() const;
	int64_t get_last_frame_checksum() const;
	Dictionary get_router_status() const;

	void set_source_name(const String &p_name);
	String get_source_name() const;
	void set_viewport_path(const NodePath &p_path);
	NodePath get_viewport_path() const;
	void set_enabled(bool p_enabled);
	bool get_enabled() const;
	void set_quality(int p_quality);
	int get_quality() const;
	void set_frame_rate(int p_frame_rate);
	int get_frame_rate() const;
	void set_include_alpha(bool p_enabled);
	bool get_include_alpha() const;
	void set_metadata(const String &p_metadata);
	String get_metadata() const;
	void send_metadata(const String &p_metadata);
};

} // namespace godot
