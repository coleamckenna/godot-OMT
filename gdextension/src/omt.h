#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class OMT : public Object {
	GDCLASS(OMT, Object);

protected:
	static void _bind_methods();

public:
	static bool is_available();
	static bool is_stub_build();
	static String get_runtime_error();
	static String get_version();
	static Dictionary get_runtime_status();
};

} // namespace godot
