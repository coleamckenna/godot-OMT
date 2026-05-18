#include "omt_output.h"

#include "omt_viewport_texture_router.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <algorithm>
#include <cstring>

#ifndef GODOT_OMT_NO_LIBOMT
#include <libomt.h>
#endif

using namespace godot;

OMTOutput::OMTOutput() {
}

OMTOutput::~OMTOutput() {
	stop();
}

void OMTOutput::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start"), &OMTOutput::start);
	ClassDB::bind_method(D_METHOD("stop"), &OMTOutput::stop);
	ClassDB::bind_method(D_METHOD("is_running"), &OMTOutput::is_running);
	ClassDB::bind_method(D_METHOD("get_address"), &OMTOutput::get_address);
	ClassDB::bind_method(D_METHOD("get_last_error"), &OMTOutput::get_last_error);
	ClassDB::bind_method(D_METHOD("get_frames_sent"), &OMTOutput::get_frames_sent);
	ClassDB::bind_method(D_METHOD("get_frames_attempted"), &OMTOutput::get_frames_attempted);
	ClassDB::bind_method(D_METHOD("get_last_send_result"), &OMTOutput::get_last_send_result);
	ClassDB::bind_method(D_METHOD("get_last_frame_checksum"), &OMTOutput::get_last_frame_checksum);
	ClassDB::bind_method(D_METHOD("get_router_status"), &OMTOutput::get_router_status);
	ClassDB::bind_method(D_METHOD("send_metadata", "metadata"), &OMTOutput::send_metadata);

	ClassDB::bind_method(D_METHOD("set_source_name", "name"), &OMTOutput::set_source_name);
	ClassDB::bind_method(D_METHOD("get_source_name"), &OMTOutput::get_source_name);
	ClassDB::bind_method(D_METHOD("set_viewport_path", "path"), &OMTOutput::set_viewport_path);
	ClassDB::bind_method(D_METHOD("get_viewport_path"), &OMTOutput::get_viewport_path);
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &OMTOutput::set_enabled);
	ClassDB::bind_method(D_METHOD("get_enabled"), &OMTOutput::get_enabled);
	ClassDB::bind_method(D_METHOD("set_quality", "quality"), &OMTOutput::set_quality);
	ClassDB::bind_method(D_METHOD("get_quality"), &OMTOutput::get_quality);
	ClassDB::bind_method(D_METHOD("set_frame_rate", "frame_rate"), &OMTOutput::set_frame_rate);
	ClassDB::bind_method(D_METHOD("get_frame_rate"), &OMTOutput::get_frame_rate);
	ClassDB::bind_method(D_METHOD("set_include_alpha", "enabled"), &OMTOutput::set_include_alpha);
	ClassDB::bind_method(D_METHOD("get_include_alpha"), &OMTOutput::get_include_alpha);
	ClassDB::bind_method(D_METHOD("set_metadata", "metadata"), &OMTOutput::set_metadata);
	ClassDB::bind_method(D_METHOD("get_metadata"), &OMTOutput::get_metadata);
	ClassDB::bind_method(D_METHOD("_send_texture", "data", "format", "viewport_rid"), &OMTOutput::_send_texture);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "source_name"), "set_source_name", "get_source_name");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "viewport_path"), "set_viewport_path", "get_viewport_path");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "quality", PROPERTY_HINT_ENUM, "Default:0,Low:1,Medium:50,High:100"), "set_quality", "get_quality");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_rate", PROPERTY_HINT_RANGE, "1,240,1"), "set_frame_rate", "get_frame_rate");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "include_alpha"), "set_include_alpha", "get_include_alpha");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "metadata", PROPERTY_HINT_MULTILINE_TEXT), "set_metadata", "get_metadata");

	ADD_SIGNAL(MethodInfo("started"));
	ADD_SIGNAL(MethodInfo("stopped"));
	ADD_SIGNAL(MethodInfo("frame_sent"));
	ADD_SIGNAL(MethodInfo("metadata_sent", PropertyInfo(Variant::STRING, "metadata")));
	ADD_SIGNAL(MethodInfo("error", PropertyInfo(Variant::STRING, "message")));
}

void OMTOutput::_ready() {
	set_process(enabled);
	if (enabled && !is_running()) {
		start();
	} else if (enabled && is_running() && registered_viewport == nullptr) {
		_register_viewport_router();
	}
}

void OMTOutput::_process(double p_delta) {
	(void)p_delta;
	if (enabled && is_running() && registered_viewport == nullptr) {
		_register_viewport_router();
	}
}

void OMTOutput::_exit_tree() {
	stop();
}

void OMTOutput::start() {
	if (is_running()) {
		return;
	}
	last_error = "";
	_create_sender();
	if (is_running()) {
		emit_signal("started");
	}
}

void OMTOutput::stop() {
	if (!is_running()) {
		return;
	}
	_destroy_sender();
	emit_signal("stopped");
}

bool OMTOutput::is_running() const {
#ifndef GODOT_OMT_NO_LIBOMT
	return omt_send_handle != nullptr;
#else
	return false;
#endif
}

String OMTOutput::get_address() const {
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_send_handle != nullptr) {
		char address[OMT_MAX_STRING_LENGTH] = {};
		omt_send_getaddress(static_cast<omt_send_t *>(omt_send_handle), address, OMT_MAX_STRING_LENGTH);
		return String(address);
	}
#endif
	return "";
}

String OMTOutput::get_last_error() const {
	return last_error;
}

int64_t OMTOutput::get_frames_sent() const {
	return frames_sent;
}

int64_t OMTOutput::get_frames_attempted() const {
	return frames_attempted;
}

int OMTOutput::get_last_send_result() const {
	return last_send_result;
}

int64_t OMTOutput::get_last_frame_checksum() const {
	return last_frame_checksum;
}

Dictionary OMTOutput::get_router_status() const {
	Dictionary status;
	status["callbacks_seen_by_output"] = readback_callbacks;
	status["registered_viewport"] = registered_viewport != nullptr;

	Engine *engine = Engine::get_singleton();
	if (engine == nullptr) {
		status["last_error"] = "Engine singleton unavailable.";
		return status;
	}

	OMTViewportTextureRouter *router = Object::cast_to<OMTViewportTextureRouter>(engine->get_singleton("OMTViewportTextureRouter"));
	if (router == nullptr) {
		status["last_error"] = "OMTViewportTextureRouter singleton unavailable.";
		return status;
	}

	status.merge(router->get_status(), true);
	return status;
}

void OMTOutput::set_source_name(const String &p_name) {
	source_name = p_name;
	if (is_running()) {
		stop();
		start();
	}
}

String OMTOutput::get_source_name() const {
	return source_name;
}

void OMTOutput::set_viewport_path(const NodePath &p_path) {
	const bool was_running = is_running();
	if (was_running) {
		_unregister_viewport_router();
	}
	viewport_path = p_path;
	if (was_running) {
		_register_viewport_router();
	}
}

NodePath OMTOutput::get_viewport_path() const {
	return viewport_path;
}

void OMTOutput::set_enabled(bool p_enabled) {
	enabled = p_enabled;
	set_process(enabled);
	if (enabled && is_inside_tree()) {
		start();
	} else {
		stop();
	}
}

bool OMTOutput::get_enabled() const {
	return enabled;
}

void OMTOutput::set_quality(int p_quality) {
	quality = p_quality;
}

int OMTOutput::get_quality() const {
	return quality;
}

void OMTOutput::set_frame_rate(int p_frame_rate) {
	frame_rate = std::max(p_frame_rate, 1);
}

int OMTOutput::get_frame_rate() const {
	return frame_rate;
}

void OMTOutput::set_include_alpha(bool p_enabled) {
	include_alpha = p_enabled;
}

bool OMTOutput::get_include_alpha() const {
	return include_alpha;
}

void OMTOutput::set_metadata(const String &p_metadata) {
	metadata = p_metadata;
}

String OMTOutput::get_metadata() const {
	return metadata;
}

void OMTOutput::send_metadata(const String &p_metadata) {
	_send_metadata_frame(p_metadata);
}

void OMTOutput::_create_sender() {
#ifndef GODOT_OMT_NO_LIBOMT
	const CharString name_utf8 = source_name.utf8();
	omt_send_t *handle = omt_send_create(name_utf8.get_data(), static_cast<OMTQuality>(quality));
	if (handle == nullptr) {
		last_error = "omt_send_create failed";
		emit_signal("error", last_error);
		return;
	}
	omt_send_handle = handle;
	_register_viewport_router();
	if (!metadata.is_empty()) {
		_send_metadata_frame(metadata);
	}
#else
	last_error = "This build was compiled without libomt";
	emit_signal("error", last_error);
#endif
}

void OMTOutput::_destroy_sender() {
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_send_handle != nullptr) {
		_unregister_viewport_router();
		omt_send_destroy(static_cast<omt_send_t *>(omt_send_handle));
		omt_send_handle = nullptr;
	}
#endif
}

Viewport *OMTOutput::_get_target_viewport() {
	Viewport *viewport = nullptr;
	if (!viewport_path.is_empty()) {
		if (!has_node(viewport_path)) {
			last_error = "viewport_path does not resolve: " + String(viewport_path);
			return nullptr;
		}
		viewport = Object::cast_to<Viewport>(get_node_or_null(viewport_path));
		if (viewport == nullptr) {
			last_error = "viewport_path is not a Viewport: " + String(viewport_path);
			return nullptr;
		}
	} else {
		viewport = get_viewport();
	}
	return viewport;
}

void OMTOutput::_register_viewport_router() {
#ifndef GODOT_OMT_NO_LIBOMT
	if (!is_inside_tree()) {
		return;
	}
	if (registered_viewport != nullptr) {
		return;
	}

	Viewport *viewport = _get_target_viewport();
	if (viewport == nullptr) {
		return;
	}

	if (SubViewport *sub_viewport = Object::cast_to<SubViewport>(viewport)) {
		sub_viewport->set_update_mode(SubViewport::UPDATE_ALWAYS);
	}
	registered_viewport = viewport;

	Engine *engine = Engine::get_singleton();
	ERR_FAIL_NULL_MSG(engine, "OMTOutput: Engine singleton unavailable.");
	OMTViewportTextureRouter *router = Object::cast_to<OMTViewportTextureRouter>(engine->get_singleton("OMTViewportTextureRouter"));
	ERR_FAIL_NULL_MSG(router, "OMTOutput: OMTViewportTextureRouter singleton unavailable.");

	Callable callback = callable_mp(this, &OMTOutput::_send_texture);
	if (!router->is_connected("texture_arrived", callback)) {
		const Error err = router->connect("texture_arrived", callback);
		if (err != OK) {
			last_error = "Could not connect to OMTViewportTextureRouter.texture_arrived.";
			emit_signal("error", last_error);
			return;
		}
	}
	router->add_viewport(viewport);
#endif
}

void OMTOutput::_unregister_viewport_router() {
#ifndef GODOT_OMT_NO_LIBOMT
	Viewport *viewport = registered_viewport;
	registered_viewport = nullptr;
	Engine *engine = Engine::get_singleton();
	if (engine == nullptr) {
		return;
	}

	OMTViewportTextureRouter *router = Object::cast_to<OMTViewportTextureRouter>(engine->get_singleton("OMTViewportTextureRouter"));
	if (router == nullptr) {
		return;
	}

	Callable callback = callable_mp(this, &OMTOutput::_send_texture);
	if (router->is_connected("texture_arrived", callback)) {
		router->disconnect("texture_arrived", callback);
	}
	if (viewport != nullptr) {
		router->remove_viewport(viewport);
	}
#endif
}

void OMTOutput::_send_texture(PackedByteArray p_data, const Ref<RDTextureFormat> &p_format, int64_t p_viewport_rid) {
#ifndef GODOT_OMT_NO_LIBOMT
	readback_callbacks++;

	if (omt_send_handle == nullptr || p_format.is_null() || p_data.is_empty()) {
		if (p_format.is_null()) {
			last_error = "Async viewport readback returned a null texture format.";
		} else if (p_data.is_empty()) {
			last_error = "Async viewport readback returned empty data.";
		}
		return;
	}

	Viewport *viewport = _get_target_viewport();
	if (viewport == nullptr || viewport->get_viewport_rid().get_id() != p_viewport_rid) {
		return;
	}

	const int width = static_cast<int>(p_format->get_width());
	const int height = static_cast<int>(p_format->get_height());
	if (width <= 0 || height <= 0 || p_data.size() != static_cast<int64_t>(width) * height * 4) {
		last_error = "Unexpected viewport texture size.";
		emit_signal("error", last_error);
		return;
	}

	const int pixel_count = width * height;
	send_bgra.resize(static_cast<size_t>(pixel_count) * 4);

	const RenderingDevice::DataFormat data_format = p_format->get_format();
	if (data_format == RenderingDevice::DATA_FORMAT_R8G8B8A8_UNORM || data_format == RenderingDevice::DATA_FORMAT_R8G8B8A8_SRGB) {
		_rgba_to_bgra(p_data.ptr(), send_bgra.data(), static_cast<size_t>(pixel_count));
	} else if (data_format == RenderingDevice::DATA_FORMAT_B8G8R8A8_UNORM || data_format == RenderingDevice::DATA_FORMAT_B8G8R8A8_SRGB) {
		std::memcpy(send_bgra.data(), p_data.ptr(), send_bgra.size());
	} else {
		last_error = "Unsupported viewport texture format: " + String::num_int64(static_cast<int64_t>(data_format));
		emit_signal("error", last_error);
		return;
	}

	uint64_t checksum = 1469598103934665603ULL;
	for (uint8_t byte : send_bgra) {
		checksum ^= byte;
		checksum *= 1099511628211ULL;
	}
	last_frame_checksum = static_cast<int64_t>(checksum);

	OMTMediaFrame frame = {};
	frame.Type = OMTFrameType_Video;
	frame.Timestamp = -1;
	frame.Codec = OMTCodec_BGRA;
	frame.Width = width;
	frame.Height = height;
	frame.Stride = width * 4;
	frame.Flags = include_alpha ? OMTVideoFlags_Alpha : OMTVideoFlags_None;
	frame.FrameRateN = frame_rate;
	frame.FrameRateD = 1;
	frame.AspectRatio = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
	frame.Data = send_bgra.data();
	frame.DataLength = static_cast<int>(send_bgra.size());

	frames_attempted++;
	last_send_result = omt_send(static_cast<omt_send_t *>(omt_send_handle), &frame);
	if (last_send_result == 0) {
		// libomt follows C-style success codes here: 0 means the frame was accepted.
		frames_sent++;
		emit_signal("frame_sent");
	}
#endif
}

void OMTOutput::_send_metadata_frame(const String &p_metadata) {
#ifndef GODOT_OMT_NO_LIBOMT
	if (omt_send_handle == nullptr || p_metadata.is_empty()) {
		return;
	}
	const CharString metadata_utf8 = p_metadata.utf8();
	OMTMediaFrame frame = {};
	frame.Type = OMTFrameType_Metadata;
	frame.Timestamp = -1;
	frame.Data = const_cast<char *>(metadata_utf8.get_data());
	frame.DataLength = static_cast<int>(metadata_utf8.length()) + 1;
	if (omt_send(static_cast<omt_send_t *>(omt_send_handle), &frame) != 0) {
		emit_signal("metadata_sent", p_metadata);
	}
#endif
}

void OMTOutput::_rgba_to_bgra(const uint8_t *p_src, uint8_t *p_dst, size_t p_pixel_count) {
	for (size_t i = 0; i < p_pixel_count; ++i) {
		const size_t offset = i * 4;
		p_dst[offset + 0] = p_src[offset + 2];
		p_dst[offset + 1] = p_src[offset + 1];
		p_dst[offset + 2] = p_src[offset + 0];
		p_dst[offset + 3] = p_src[offset + 3];
	}
}
