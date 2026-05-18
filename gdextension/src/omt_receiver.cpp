#include "omt_receiver.h"

#include "omt_frame_converter.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>
#include <cstring>

#ifndef GODOT_OMT_NO_LIBOMT
#include <libomt.h>
#endif

using namespace godot;
using namespace godot_omt;

OMTReceiver::OMTReceiver() {
	buffer_mutex.instantiate();
	texture.instantiate();
	image = Image::create(2, 2, false, Image::FORMAT_RGBA8);
	texture->set_image(image);
}

OMTReceiver::~OMTReceiver() {
	stop();
}

void OMTReceiver::_bind_methods() {
	ClassDB::bind_static_method("OMTReceiver", D_METHOD("discover_sources"), &OMTReceiver::discover_sources);

	BIND_ENUM_CONSTANT(STATE_DISCONNECTED);
	BIND_ENUM_CONSTANT(STATE_CONNECTING);
	BIND_ENUM_CONSTANT(STATE_CONNECTED);
	BIND_ENUM_CONSTANT(STATE_ERROR);

	ClassDB::bind_method(D_METHOD("start"), &OMTReceiver::start);
	ClassDB::bind_method(D_METHOD("stop"), &OMTReceiver::stop);
	ClassDB::bind_method(D_METHOD("get_texture"), &OMTReceiver::get_texture);
	ClassDB::bind_method(D_METHOD("has_new_frame"), &OMTReceiver::has_new_frame);
	ClassDB::bind_method(D_METHOD("clear_new_frame_flag"), &OMTReceiver::clear_new_frame_flag);
	ClassDB::bind_method(D_METHOD("is_omt_connected"), &OMTReceiver::is_omt_connected);
	ClassDB::bind_method(D_METHOD("get_connection_state"), &OMTReceiver::get_connection_state);
	ClassDB::bind_method(D_METHOD("get_last_error"), &OMTReceiver::get_last_error);
	ClassDB::bind_method(D_METHOD("get_last_metadata"), &OMTReceiver::get_last_metadata);
	ClassDB::bind_method(D_METHOD("get_last_audio_info"), &OMTReceiver::get_last_audio_info);
	ClassDB::bind_method(D_METHOD("get_video_statistics"), &OMTReceiver::get_video_statistics);
	ClassDB::bind_method(D_METHOD("get_audio_statistics"), &OMTReceiver::get_audio_statistics);
	ClassDB::bind_method(D_METHOD("get_sender_information"), &OMTReceiver::get_sender_information);
	ClassDB::bind_method(D_METHOD("get_display_statistics"), &OMTReceiver::get_display_statistics);
	ClassDB::bind_method(D_METHOD("refresh_sources"), &OMTReceiver::refresh_sources);

	ClassDB::bind_method(D_METHOD("set_source_address", "address"), &OMTReceiver::set_source_address);
	ClassDB::bind_method(D_METHOD("get_source_address"), &OMTReceiver::get_source_address);
	ClassDB::bind_method(D_METHOD("set_auto_start", "enabled"), &OMTReceiver::set_auto_start);
	ClassDB::bind_method(D_METHOD("get_auto_start"), &OMTReceiver::get_auto_start);
	ClassDB::bind_method(D_METHOD("set_receive_timeout_ms", "timeout_ms"), &OMTReceiver::set_receive_timeout_ms);
	ClassDB::bind_method(D_METHOD("get_receive_timeout_ms"), &OMTReceiver::get_receive_timeout_ms);
	ClassDB::bind_method(D_METHOD("set_use_test_pattern", "enabled"), &OMTReceiver::set_use_test_pattern);
	ClassDB::bind_method(D_METHOD("get_use_test_pattern"), &OMTReceiver::get_use_test_pattern);
	ClassDB::bind_method(D_METHOD("set_test_pattern_texture", "texture"), &OMTReceiver::set_test_pattern_texture);
	ClassDB::bind_method(D_METHOD("get_test_pattern_texture"), &OMTReceiver::get_test_pattern_texture);
	ClassDB::bind_method(D_METHOD("set_preferred_format", "format"), &OMTReceiver::set_preferred_format);
	ClassDB::bind_method(D_METHOD("get_preferred_format"), &OMTReceiver::get_preferred_format);
	ClassDB::bind_method(D_METHOD("set_receive_flags", "flags"), &OMTReceiver::set_receive_flags);
	ClassDB::bind_method(D_METHOD("get_receive_flags"), &OMTReceiver::get_receive_flags);
	ClassDB::bind_method(D_METHOD("set_quality", "quality"), &OMTReceiver::set_quality);
	ClassDB::bind_method(D_METHOD("get_quality"), &OMTReceiver::get_quality);
	ClassDB::bind_method(D_METHOD("set_preview_mode", "enabled"), &OMTReceiver::set_preview_mode);
	ClassDB::bind_method(D_METHOD("get_preview_mode"), &OMTReceiver::get_preview_mode);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "source_address"), "set_source_address", "get_source_address");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_start"), "set_auto_start", "get_auto_start");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "receive_timeout_ms"), "set_receive_timeout_ms", "get_receive_timeout_ms");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_test_pattern"), "set_use_test_pattern", "get_use_test_pattern");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "test_pattern_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_test_pattern_texture", "get_test_pattern_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "preferred_format", PROPERTY_HINT_ENUM, "UYVY,UYVY or BGRA,BGRA,UYVY or UYVA,UYVY or UYVA or P216 or PA16,P216"), "set_preferred_format", "get_preferred_format");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "receive_flags", PROPERTY_HINT_FLAGS, "Preview,Include Compressed,Compressed Only"), "set_receive_flags", "get_receive_flags");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "quality", PROPERTY_HINT_ENUM, "Default:0,Low:1,Medium:50,High:100"), "set_quality", "get_quality");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "preview_mode"), "set_preview_mode", "get_preview_mode");

	ADD_SIGNAL(MethodInfo("frame_ready"));
	ADD_SIGNAL(MethodInfo("connected"));
	ADD_SIGNAL(MethodInfo("disconnected"));
	ADD_SIGNAL(MethodInfo("format_changed"));
	ADD_SIGNAL(MethodInfo("statistics_changed"));
	ADD_SIGNAL(MethodInfo("metadata_received", PropertyInfo(Variant::STRING, "metadata")));
	ADD_SIGNAL(MethodInfo("audio_frame_received", PropertyInfo(Variant::DICTIONARY, "audio_info")));
	ADD_SIGNAL(MethodInfo("connection_state_changed", PropertyInfo(Variant::INT, "state")));
}

void OMTReceiver::_validate_property(PropertyInfo &p_property) const {
	if (String(p_property.name) != "source_address") {
		return;
	}

	_refresh_source_cache();
	if (source_dropdown_cache.is_empty()) {
		return;
	}

	String hint_string;
	for (int64_t i = 0; i < source_dropdown_cache.size(); ++i) {
		if (i > 0) {
			hint_string += ",";
		}
		hint_string += source_dropdown_cache[i];
	}

	p_property.hint = PROPERTY_HINT_ENUM;
	p_property.hint_string = hint_string;
}

void OMTReceiver::_ready() {
	set_process(true);
	if (auto_start) {
		start();
	}
}

void OMTReceiver::_process(double /*delta*/) {
	if (!frame_pending.load()) {
		if (metadata_pending) {
			buffer_mutex->lock();
			String metadata = last_metadata;
			metadata_pending = false;
			buffer_mutex->unlock();
			emit_signal("metadata_received", metadata);
		}
		if (audio_pending) {
			buffer_mutex->lock();
			Dictionary audio_info = last_audio_info;
			audio_pending = false;
			buffer_mutex->unlock();
			emit_signal("audio_frame_received", audio_info);
		}
		return;
	}
	_apply_pending_frame();
	frame_pending.store(false);
	emit_signal("frame_ready");
	if (metadata_pending) {
		buffer_mutex->lock();
		String metadata = last_metadata;
		metadata_pending = false;
		buffer_mutex->unlock();
		emit_signal("metadata_received", metadata);
	}
	if (audio_pending) {
		buffer_mutex->lock();
		Dictionary audio_info = last_audio_info;
		audio_pending = false;
		buffer_mutex->unlock();
		emit_signal("audio_frame_received", audio_info);
	}
}

void OMTReceiver::_exit_tree() {
	stop();
}

void OMTReceiver::set_source_address(const String &p_address) {
	source_address = p_address;
}

String OMTReceiver::get_source_address() const {
	return source_address;
}

void OMTReceiver::set_auto_start(bool p_enabled) {
	auto_start = p_enabled;
}

bool OMTReceiver::get_auto_start() const {
	return auto_start;
}

void OMTReceiver::set_receive_timeout_ms(int p_ms) {
	receive_timeout_ms = MAX(p_ms, 1);
}

int OMTReceiver::get_receive_timeout_ms() const {
	return receive_timeout_ms;
}

void OMTReceiver::set_use_test_pattern(bool p_enabled) {
	use_test_pattern = p_enabled;
}

bool OMTReceiver::get_use_test_pattern() const {
	return use_test_pattern;
}

void OMTReceiver::set_test_pattern_texture(const Ref<Texture2D> &p_texture) {
	test_pattern_texture = p_texture;
	_rebuild_test_pattern_cache();
}

Ref<Texture2D> OMTReceiver::get_test_pattern_texture() const {
	return test_pattern_texture;
}

void OMTReceiver::set_preferred_format(int p_format) {
	preferred_format = p_format;
}

int OMTReceiver::get_preferred_format() const {
	return preferred_format;
}

void OMTReceiver::set_receive_flags(int p_flags) {
	receive_flags = p_flags;
	preview_mode = (receive_flags & 1) != 0;
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_receive_handle != nullptr) {
		omt_receive_setflags(static_cast<omt_receive_t *>(omt_receive_handle), static_cast<OMTReceiveFlags>(receive_flags));
	}
#endif
}

int OMTReceiver::get_receive_flags() const {
	return receive_flags;
}

void OMTReceiver::set_quality(int p_quality) {
	quality = p_quality;
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_receive_handle != nullptr) {
		omt_receive_setsuggestedquality(static_cast<omt_receive_t *>(omt_receive_handle), static_cast<OMTQuality>(quality));
	}
#endif
}

int OMTReceiver::get_quality() const {
	return quality;
}

void OMTReceiver::set_preview_mode(bool p_enabled) {
	preview_mode = p_enabled;
	if (preview_mode) {
		receive_flags |= 1;
	} else {
		receive_flags &= ~1;
	}
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_receive_handle != nullptr) {
		omt_receive_setflags(static_cast<omt_receive_t *>(omt_receive_handle), static_cast<OMTReceiveFlags>(receive_flags));
	}
#endif
}

bool OMTReceiver::get_preview_mode() const {
	return preview_mode;
}

Ref<Texture2D> OMTReceiver::get_texture() const {
	return texture;
}

bool OMTReceiver::has_new_frame() const {
	return frame_pending.load();
}

void OMTReceiver::clear_new_frame_flag() {
	frame_pending.store(false);
}

bool OMTReceiver::is_omt_connected() const {
	return connected.load();
}

OMTReceiver::ConnectionState OMTReceiver::get_connection_state() const {
	return connection_state;
}

String OMTReceiver::get_last_error() const {
	return last_error;
}

String OMTReceiver::get_last_metadata() const {
	return last_metadata;
}

Dictionary OMTReceiver::get_last_audio_info() const {
	return last_audio_info;
}

Dictionary OMTReceiver::get_video_statistics() const {
	Dictionary result;
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_receive_handle != nullptr) {
		OMTStatistics stats = {};
		omt_receive_getvideostatistics(static_cast<omt_receive_t *>(omt_receive_handle), &stats);
		result["bytes_received"] = stats.BytesReceived;
		result["bytes_received_since_last"] = stats.BytesReceivedSinceLast;
		result["frames"] = stats.Frames;
		result["frames_since_last"] = stats.FramesSinceLast;
		result["frames_dropped"] = stats.FramesDropped;
		result["codec_time"] = stats.CodecTime;
		result["codec_time_since_last"] = stats.CodecTimeSinceLast;
	}
#endif
	return result;
}

Dictionary OMTReceiver::get_audio_statistics() const {
	Dictionary result;
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_receive_handle != nullptr) {
		OMTStatistics stats = {};
		omt_receive_getaudiostatistics(static_cast<omt_receive_t *>(omt_receive_handle), &stats);
		result["bytes_received"] = stats.BytesReceived;
		result["bytes_received_since_last"] = stats.BytesReceivedSinceLast;
		result["frames"] = stats.Frames;
		result["frames_since_last"] = stats.FramesSinceLast;
		result["frames_dropped"] = stats.FramesDropped;
		result["codec_time"] = stats.CodecTime;
		result["codec_time_since_last"] = stats.CodecTimeSinceLast;
	}
#endif
	return result;
}

Dictionary OMTReceiver::get_sender_information() const {
	Dictionary result;
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_receive_handle != nullptr) {
		OMTSenderInfo info = {};
		omt_receive_getsenderinformation(static_cast<omt_receive_t *>(omt_receive_handle), &info);
		result["product_name"] = String(info.ProductName);
		result["manufacturer"] = String(info.Manufacturer);
		result["version"] = String(info.Version);
	}
#endif
	return result;
}

Dictionary OMTReceiver::get_display_statistics() const {
	Dictionary result;
	result["frames_applied"] = frames_applied;
	result["last_applied_checksum"] = last_applied_checksum;
	result["texture_width"] = image.is_valid() ? image->get_width() : 0;
	result["texture_height"] = image.is_valid() ? image->get_height() : 0;
	result["frame_pending"] = frame_pending.load();
	return result;
}

void OMTReceiver::refresh_sources() {
	source_dropdown_cache = discover_sources();
	source_dropdown_cache_initialized = true;
	notify_property_list_changed();
}

PackedStringArray OMTReceiver::discover_sources() {
	PackedStringArray result;
#ifndef GODOT_OMT_NO_LIBOMT
	int count = 0;
	char **addresses = omt_discovery_getaddresses(&count);
	if (addresses == nullptr || count <= 0) {
		return result;
	}
	for (int i = 0; i < count; ++i) {
		if (addresses[i] != nullptr) {
			result.push_back(String(addresses[i]));
		}
	}
#else
	UtilityFunctions::push_warning("OMTReceiver: built without libomt; discovery unavailable.");
#endif
	return result;
}

void OMTReceiver::_refresh_source_cache() const {
	if (source_dropdown_cache_initialized) {
		return;
	}
	source_dropdown_cache = discover_sources();
	source_dropdown_cache_initialized = true;
}

void OMTReceiver::start() {
	if (running.load()) {
		return;
	}

	last_error = "";
	_set_connection_state(STATE_CONNECTING);

#ifndef GODOT_OMT_NO_LIBOMT
	if (!use_test_pattern) {
		if (source_address.is_empty()) {
			last_error = "source_address is empty";
			UtilityFunctions::push_error("OMTReceiver: source_address is empty.");
			_set_connection_state(STATE_ERROR);
			return;
		}

		const CharString address_utf8 = source_address.utf8();
		omt_receive_t *handle = omt_receive_create(
				address_utf8.get_data(),
				static_cast<OMTFrameType>(OMTFrameType_Video | OMTFrameType_Audio | OMTFrameType_Metadata),
				static_cast<OMTPreferredVideoFormat>(preferred_format),
				static_cast<OMTReceiveFlags>(receive_flags));

		if (handle == nullptr) {
			last_error = "omt_receive_create failed for address: " + source_address;
			UtilityFunctions::push_error("OMTReceiver: " + last_error);
			_set_connection_state(STATE_ERROR);
			return;
		}

		omt_receive_handle = handle;
		omt_receive_setsuggestedquality(handle, static_cast<OMTQuality>(quality));
		connected.store(true);
		_set_connection_state(STATE_CONNECTED);
		emit_signal("connected");
	}
#else
	if (!use_test_pattern) {
		last_error = "libomt not linked; enabling test pattern";
		UtilityFunctions::push_warning("OMTReceiver: libomt not linked; enabling test pattern.");
		use_test_pattern = true;
	}
#endif

	if (use_test_pattern) {
		connected.store(true);
		_set_connection_state(STATE_CONNECTED);
	}

	running.store(true);
	receive_thread = std::make_unique<std::thread>(&OMTReceiver::_receive_loop, this);
}

void OMTReceiver::stop() {
	if (!running.load()) {
		return;
	}

	running.store(false);

	if (receive_thread && receive_thread->joinable()) {
		receive_thread->join();
	}
	receive_thread.reset();

#ifndef GODOT_OMT_NO_LIBOMT
	_destroy_omt();
#else
	if (connected.exchange(false)) {
		emit_signal("disconnected");
	}
	_set_connection_state(STATE_DISCONNECTED);
#endif
}

#ifndef GODOT_OMT_NO_LIBOMT
void OMTReceiver::_destroy_omt() {
	if (omt_receive_handle != nullptr) {
		omt_receive_destroy(static_cast<omt_receive_t *>(omt_receive_handle));
		omt_receive_handle = nullptr;
	}
	if (connected.exchange(false)) {
		call_deferred("emit_signal", "disconnected");
	}
	_set_connection_state(STATE_DISCONNECTED);
}
#endif

void OMTReceiver::_set_connection_state(ConnectionState p_state) {
	if (connection_state == p_state) {
		return;
	}
	connection_state = p_state;
	emit_signal("connection_state_changed", static_cast<int>(connection_state));
}

void OMTReceiver::_receive_loop() {
	while (running.load()) {
		std::vector<uint8_t> rgba;
		int w = 0;
		int h = 0;

		if (!_poll_frame(rgba, w, h)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		buffer_mutex->lock();
		frame_width = w;
		frame_height = h;
		pending_rgba = std::move(rgba);
		buffer_mutex->unlock();

		frame_pending.store(true);
	}
}

bool OMTReceiver::_poll_frame(std::vector<uint8_t> &out_rgba, int &out_w, int &out_h) {
	if (use_test_pattern) {
		return _poll_test_pattern(out_rgba, out_w, out_h);
	}
#ifndef GODOT_OMT_NO_LIBOMT
	return _poll_libomt(out_rgba, out_w, out_h);
#else
	return _poll_test_pattern(out_rgba, out_w, out_h);
#endif
}

bool OMTReceiver::_poll_test_pattern(std::vector<uint8_t> &out_rgba, int &out_w, int &out_h) {
	buffer_mutex->lock();
	if (!test_pattern_rgba.empty() && test_pattern_width > 0 && test_pattern_height > 0) {
		out_w = test_pattern_width;
		out_h = test_pattern_height;
		out_rgba = test_pattern_rgba;
		buffer_mutex->unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		return true;
	}
	buffer_mutex->unlock();

	static uint8_t t = 0;
	t++;

	out_w = 64;
	out_h = 64;
	out_rgba.resize(static_cast<size_t>(out_w) * static_cast<size_t>(out_h) * 4);

	for (int y = 0; y < out_h; ++y) {
		for (int x = 0; x < out_w; ++x) {
			const size_t i = (static_cast<size_t>(y) * static_cast<size_t>(out_w) + static_cast<size_t>(x)) * 4;
			out_rgba[i + 0] = static_cast<uint8_t>(x * 4);
			out_rgba[i + 1] = static_cast<uint8_t>(y * 4);
			out_rgba[i + 2] = t;
			out_rgba[i + 3] = 255;
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(16));
	return true;
}

void OMTReceiver::_rebuild_test_pattern_cache() {
	std::vector<uint8_t> rgba;
	int w = 0;
	int h = 0;

	if (test_pattern_texture.is_valid()) {
		Ref<Image> source_image = test_pattern_texture->get_image();
		if (source_image.is_valid() && source_image->get_width() > 0 && source_image->get_height() > 0) {
			source_image->convert(Image::FORMAT_RGBA8);
			PackedByteArray data = source_image->get_data();
			if (!data.is_empty()) {
				w = source_image->get_width();
				h = source_image->get_height();
				const int64_t size = data.size();
				rgba.resize(static_cast<size_t>(size));
				memcpy(rgba.data(), data.ptr(), static_cast<size_t>(size));
			}
		}
	}

	buffer_mutex->lock();
	test_pattern_width = w;
	test_pattern_height = h;
	test_pattern_rgba = std::move(rgba);
	buffer_mutex->unlock();
}

#ifndef GODOT_OMT_NO_LIBOMT
bool OMTReceiver::_poll_libomt(std::vector<uint8_t> &out_rgba, int &out_w, int &out_h) {
	if (omt_receive_handle == nullptr) {
		return false;
	}

	OMTMediaFrame *frame = omt_receive(
			static_cast<omt_receive_t *>(omt_receive_handle),
			static_cast<OMTFrameType>(OMTFrameType_Video | OMTFrameType_Audio | OMTFrameType_Metadata),
			receive_timeout_ms);

	if (frame == nullptr) {
		return false;
	}

	if (frame->Type == OMTFrameType_Metadata && frame->Data != nullptr && frame->DataLength > 0) {
		buffer_mutex->lock();
		last_metadata = String(static_cast<const char *>(frame->Data));
		metadata_pending = true;
		buffer_mutex->unlock();
		return false;
	}

	if (frame->Type == OMTFrameType_Audio) {
		Dictionary audio_info;
		audio_info["sample_rate"] = frame->SampleRate;
		audio_info["channels"] = frame->Channels;
		audio_info["samples_per_channel"] = frame->SamplesPerChannel;
		audio_info["data_length"] = frame->DataLength;
		audio_info["codec"] = static_cast<int>(frame->Codec);
		buffer_mutex->lock();
		last_audio_info = audio_info;
		audio_pending = true;
		buffer_mutex->unlock();
		return false;
	}

	if (frame->Type != OMTFrameType_Video) {
		return false;
	}

	out_w = frame->Width;
	out_h = frame->Height;
	if (out_w <= 0 || out_h <= 0 || frame->Data == nullptr || frame->DataLength <= 0) {
		return false;
	}

	const size_t pixel_count = static_cast<size_t>(out_w) * static_cast<size_t>(out_h);
	out_rgba.resize(pixel_count * 4);

	switch (frame->Codec) {
		case OMTCodec_BGRA: {
			const auto *src = static_cast<const uint8_t *>(frame->Data);
			bgra_to_rgba(src, out_rgba.data(), pixel_count);
			return true;
		}
		case OMTCodec_UYVY:
		case OMTCodec_YUY2: {
			uyvy_to_rgba(
					static_cast<const uint8_t *>(frame->Data),
					out_w,
					out_h,
					frame->Stride > 0 ? frame->Stride : out_w * 2,
					out_rgba);
			return true;
		}
		default:
			UtilityFunctions::push_warning(
					"OMTReceiver: unsupported video codec for this build; use BGRA or UYVY.");
			return false;
	}
}
#endif

void OMTReceiver::_apply_pending_frame() {
	std::vector<uint8_t> rgba_copy;
	int w = 0;
	int h = 0;

	buffer_mutex->lock();
	w = frame_width;
	h = frame_height;
	rgba_copy = pending_rgba;
	buffer_mutex->unlock();

	if (w <= 0 || h <= 0 || rgba_copy.empty()) {
		return;
	}

	const int64_t expected = static_cast<int64_t>(w) * static_cast<int64_t>(h) * 4;
	if (static_cast<int64_t>(rgba_copy.size()) < expected) {
		return;
	}

	if (!image.is_valid() || image->get_width() != w || image->get_height() != h) {
		image = Image::create(w, h, false, Image::FORMAT_RGBA8);
		texture->set_image(image);
	}

	PackedByteArray packed;
	packed.resize(static_cast<int64_t>(rgba_copy.size()));
	memcpy(packed.ptrw(), rgba_copy.data(), rgba_copy.size());

	uint64_t checksum = 1469598103934665603ULL;
	for (uint8_t byte : rgba_copy) {
		checksum ^= byte;
		checksum *= 1099511628211ULL;
	}

	image->set_data(w, h, false, Image::FORMAT_RGBA8, packed);
	texture->update(image);
	frames_applied++;
	last_applied_checksum = static_cast<int64_t>(checksum);
}
