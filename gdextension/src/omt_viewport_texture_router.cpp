#include "omt_viewport_texture_router.h"

#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OMTViewportTextureRouter::OMTViewportTextureRouter() {
}

OMTViewportTextureRouter::~OMTViewportTextureRouter() {
	_disconnect_frame_signal();
}

void OMTViewportTextureRouter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_viewport", "viewport"), &OMTViewportTextureRouter::add_viewport);
	ClassDB::bind_method(D_METHOD("remove_viewport", "viewport"), &OMTViewportTextureRouter::remove_viewport);
	ClassDB::bind_method(D_METHOD("get_status"), &OMTViewportTextureRouter::get_status);

	ADD_SIGNAL(MethodInfo(
			"texture_arrived",
			PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"),
			PropertyInfo(Variant::OBJECT, "format", PROPERTY_HINT_RESOURCE_TYPE, "RDTextureFormat"),
			PropertyInfo(Variant::INT, "viewport_rid")));
}

void OMTViewportTextureRouter::add_viewport(Viewport *p_viewport) {
	if (p_viewport == nullptr) {
		_set_last_error("OMTViewportTextureRouter: viewport is null.");
		return;
	}

	const int64_t count = static_cast<int64_t>(viewports.get(p_viewport, 0));
	viewports[p_viewport] = count + 1;

	if (viewports.size() == 1) {
		_connect_frame_signal();
	}
}

void OMTViewportTextureRouter::remove_viewport(Viewport *p_viewport) {
	if (p_viewport == nullptr) {
		_set_last_error("OMTViewportTextureRouter: viewport is null.");
		return;
	}

	const int64_t count = static_cast<int64_t>(viewports.get(p_viewport, 0)) - 1;
	if (count <= 0) {
		viewports.erase(p_viewport);
	} else {
		viewports[p_viewport] = count;
	}

	if (viewports.is_empty()) {
		_disconnect_frame_signal();
	}
}

void OMTViewportTextureRouter::_connect_frame_signal() {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs == nullptr) {
		_set_last_error("OMTViewportTextureRouter: RenderingServer is unavailable.");
		return;
	}

	const Callable callable = callable_mp(this, &OMTViewportTextureRouter::_request_textures);
	if (!rs->is_connected("frame_post_draw", callable)) {
		const Error err = rs->connect("frame_post_draw", callable);
		if (err != OK) {
			_set_last_error("OMTViewportTextureRouter: could not connect frame_post_draw.");
		}
	}
}

void OMTViewportTextureRouter::_disconnect_frame_signal() {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs == nullptr) {
		return;
	}

	const Callable callable = callable_mp(this, &OMTViewportTextureRouter::_request_textures);
	if (rs->is_connected("frame_post_draw", callable)) {
		rs->disconnect("frame_post_draw", callable);
	}
}

void OMTViewportTextureRouter::_request_textures() {
	frames_observed++;

	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs == nullptr) {
		_set_last_error("OMTViewportTextureRouter: RenderingServer is unavailable.");
		return;
	}

	RenderingDevice *rd = rs->get_rendering_device();
	if (rd == nullptr) {
		_set_last_error("OMTViewportTextureRouter: RenderingDevice is unavailable.");
		return;
	}

	Array keys = viewports.keys();
	for (int64_t i = 0; i < keys.size(); ++i) {
		Viewport *viewport = Object::cast_to<Viewport>(keys[i]);
		if (viewport == nullptr) {
			_set_last_error("OMTViewportTextureRouter: tracked viewport became invalid.");
			continue;
		}

		const RID viewport_rid = viewport->get_viewport_rid();
		if (!viewport_rid.is_valid()) {
			_set_last_error("OMTViewportTextureRouter: viewport RID is invalid.");
			continue;
		}

		const RID texture_rid = rs->viewport_get_texture(viewport_rid);
		if (!texture_rid.is_valid()) {
			_set_last_error("OMTViewportTextureRouter: viewport texture RID is invalid.");
			continue;
		}

		const RID rd_texture_rid = rs->texture_get_rd_texture(texture_rid);
		if (!rd_texture_rid.is_valid()) {
			_set_last_error("OMTViewportTextureRouter: RD texture RID is invalid.");
			continue;
		}

		Ref<RDTextureFormat> texture_format = rd->texture_get_format(rd_texture_rid);
		if (texture_format.is_null()) {
			_set_last_error("OMTViewportTextureRouter: RD texture format is null.");
			continue;
		}

		Callable callback = callable_mp(this, &OMTViewportTextureRouter::_forward_texture).bind(texture_format, viewport_rid.get_id());
		const Error err = rd->texture_get_data_async(rd_texture_rid, 0, callback);
		if (err == OK) {
			readback_requests++;
		} else {
			_set_last_error("OMTViewportTextureRouter: texture_get_data_async failed.");
		}
	}
}

void OMTViewportTextureRouter::_forward_texture(PackedByteArray p_data, const Ref<RDTextureFormat> &p_format, int64_t p_viewport_rid) {
	readback_callbacks++;
	emit_signal("texture_arrived", p_data, p_format, p_viewport_rid);
}

void OMTViewportTextureRouter::_set_last_error(const String &p_error) {
	last_error = p_error;
}

Dictionary OMTViewportTextureRouter::get_status() const {
	Dictionary status;
	status["viewports"] = viewports.size();
	status["frames_observed"] = frames_observed;
	status["readback_requests"] = readback_requests;
	status["readback_callbacks"] = readback_callbacks;
	status["last_error"] = last_error;
	return status;
}
