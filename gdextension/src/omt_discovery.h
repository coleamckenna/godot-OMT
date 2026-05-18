#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

namespace godot {

class OMTDiscovery : public Node {
	GDCLASS(OMTDiscovery, Node);

private:
	PackedStringArray sources;

	static bool _contains(const PackedStringArray &p_array, const String &p_value);

protected:
	static void _bind_methods();

public:
	void _ready() override;

	void refresh();
	PackedStringArray get_sources() const;
};

} // namespace godot
