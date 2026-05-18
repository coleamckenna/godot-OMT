#include "register_types.h"

#include "omt.h"
#include "omt_discovery.h"
#include "omt_output.h"
#include "omt_receiver.h"
#include "omt_video_stream.h"
#include "omt_video_stream_playback.h"
#include "omt_viewport_texture_router.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

static OMTViewportTextureRouter *omt_viewport_texture_router = nullptr;

void initialize_godot_omt_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<OMT>();
	ClassDB::register_class<OMTDiscovery>();
	ClassDB::register_class<OMTReceiver>();
	ClassDB::register_class<OMTOutput>();
	ClassDB::register_class<OMTVideoStream>();
	ClassDB::register_class<OMTVideoStreamPlayback>();
	ClassDB::register_class<OMTViewportTextureRouter>();

	omt_viewport_texture_router = memnew(OMTViewportTextureRouter);
	Engine::get_singleton()->register_singleton("OMTViewportTextureRouter", omt_viewport_texture_router);
}

void uninitialize_godot_omt_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	if (omt_viewport_texture_router != nullptr) {
		Engine::get_singleton()->unregister_singleton("OMTViewportTextureRouter");
		memdelete(omt_viewport_texture_router);
		omt_viewport_texture_router = nullptr;
	}
}

extern "C" {

GDExtensionBool GDE_EXPORT godot_omt_library_init(
		GDExtensionInterfaceGetProcAddress p_get_proc_address,
		const GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_godot_omt_module);
	init_obj.register_terminator(uninitialize_godot_omt_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}

} // extern "C"
