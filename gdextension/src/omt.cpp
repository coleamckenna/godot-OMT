#include "omt.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void OMT::_bind_methods() {
	ClassDB::bind_static_method("OMT", D_METHOD("is_available"), &OMT::is_available);
	ClassDB::bind_static_method("OMT", D_METHOD("is_stub_build"), &OMT::is_stub_build);
	ClassDB::bind_static_method("OMT", D_METHOD("get_runtime_error"), &OMT::get_runtime_error);
	ClassDB::bind_static_method("OMT", D_METHOD("get_version"), &OMT::get_version);
	ClassDB::bind_static_method("OMT", D_METHOD("get_runtime_status"), &OMT::get_runtime_status);
}

bool OMT::is_available() {
#ifdef GODOT_OMT_NO_LIBOMT
	return false;
#else
	return true;
#endif
}

bool OMT::is_stub_build() {
	return !is_available();
}

String OMT::get_runtime_error() {
#ifdef GODOT_OMT_NO_LIBOMT
	return "This GDExtension was built without libomt. Build/stage libomt.so and libvmx.so, then rebuild.";
#else
	return "";
#endif
}

String OMT::get_version() {
	return "libomt";
}

Dictionary OMT::get_runtime_status() {
	Dictionary status;
	status["available"] = is_available();
	status["stub_build"] = is_stub_build();
	status["runtime_error"] = get_runtime_error();
	status["version"] = get_version();
	return status;
}
