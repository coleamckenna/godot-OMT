#pragma once

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace godot {

class OMTReceiver : public Node {
	GDCLASS(OMTReceiver, Node);

public:
	enum ConnectionState {
		STATE_DISCONNECTED = 0,
		STATE_CONNECTING = 1,
		STATE_CONNECTED = 2,
		STATE_ERROR = 3,
	};

protected:
	static void _bind_methods();

private:
	String source_address;
	bool auto_start = true;
	int receive_timeout_ms = 16;
	bool use_test_pattern = false;
	Ref<Texture2D> test_pattern_texture;
	int preferred_format = 2;
	int receive_flags = 0;
	int quality = 0;
	bool preview_mode = false;
	ConnectionState connection_state = STATE_DISCONNECTED;
	String last_error;
	String last_metadata;
	Dictionary last_audio_info;

	Ref<ImageTexture> texture;
	Ref<Image> image;
	Ref<Mutex> buffer_mutex;
	int64_t frames_applied = 0;
	int64_t last_applied_checksum = 0;

	std::unique_ptr<std::thread> receive_thread;
	std::atomic<bool> running{false};
	std::atomic<bool> frame_pending{false};
	std::atomic<bool> connected{false};

	std::vector<uint8_t> pending_rgba;
	int frame_width = 0;
	int frame_height = 0;
	std::vector<uint8_t> test_pattern_rgba;
	int test_pattern_width = 0;
	int test_pattern_height = 0;
	bool metadata_pending = false;
	bool audio_pending = false;
	mutable PackedStringArray source_dropdown_cache;
	mutable bool source_dropdown_cache_initialized = false;

#ifndef GODOT_OMT_NO_LIBOMT
	void *omt_receive_handle = nullptr;
#endif

	void _receive_loop();
	void _apply_pending_frame();
	bool _poll_frame(std::vector<uint8_t> &out_rgba, int &out_w, int &out_h);
	bool _poll_test_pattern(std::vector<uint8_t> &out_rgba, int &out_w, int &out_h);
	void _rebuild_test_pattern_cache();
	void _refresh_source_cache() const;
	void _set_connection_state(ConnectionState p_state);
#ifndef GODOT_OMT_NO_LIBOMT
	bool _poll_libomt(std::vector<uint8_t> &out_rgba, int &out_w, int &out_h);
	void _destroy_omt();
#endif

public:
	OMTReceiver();
	~OMTReceiver() override;

	void _validate_property(PropertyInfo &p_property) const;
	void _ready() override;
	void _process(double delta) override;
	void _exit_tree() override;

	void start();
	void stop();
	Ref<Texture2D> get_texture() const;
	bool has_new_frame() const;
	void clear_new_frame_flag();
	bool is_omt_connected() const;
	ConnectionState get_connection_state() const;
	String get_last_error() const;
	String get_last_metadata() const;
	Dictionary get_last_audio_info() const;
	Dictionary get_video_statistics() const;
	Dictionary get_audio_statistics() const;
	Dictionary get_sender_information() const;
	Dictionary get_display_statistics() const;
	void refresh_sources();

	void set_source_address(const String &p_address);
	String get_source_address() const;
	void set_auto_start(bool p_enabled);
	bool get_auto_start() const;
	void set_receive_timeout_ms(int p_ms);
	int get_receive_timeout_ms() const;
	void set_use_test_pattern(bool p_enabled);
	bool get_use_test_pattern() const;
	void set_test_pattern_texture(const Ref<Texture2D> &p_texture);
	Ref<Texture2D> get_test_pattern_texture() const;
	void set_preferred_format(int p_format);
	int get_preferred_format() const;
	void set_receive_flags(int p_flags);
	int get_receive_flags() const;
	void set_quality(int p_quality);
	int get_quality() const;
	void set_preview_mode(bool p_enabled);
	bool get_preview_mode() const;

	static PackedStringArray discover_sources();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::OMTReceiver::ConnectionState);
