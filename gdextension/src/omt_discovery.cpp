#include "omt_discovery.h"

#include "omt_receiver.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void OMTDiscovery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("refresh"), &OMTDiscovery::refresh);
	ClassDB::bind_method(D_METHOD("get_sources"), &OMTDiscovery::get_sources);

	ADD_SIGNAL(MethodInfo("source_added", PropertyInfo(Variant::STRING, "source")));
	ADD_SIGNAL(MethodInfo("source_removed", PropertyInfo(Variant::STRING, "source")));
	ADD_SIGNAL(MethodInfo("sources_changed"));
}

void OMTDiscovery::_ready() {
	refresh();
}

void OMTDiscovery::refresh() {
	PackedStringArray next_sources = OMTReceiver::discover_sources();
	bool changed = false;

	for (int64_t i = 0; i < next_sources.size(); ++i) {
		const String source = next_sources[i];
		if (!_contains(sources, source)) {
			emit_signal("source_added", source);
			changed = true;
		}
	}

	for (int64_t i = 0; i < sources.size(); ++i) {
		const String source = sources[i];
		if (!_contains(next_sources, source)) {
			emit_signal("source_removed", source);
			changed = true;
		}
	}

	sources = next_sources;
	if (changed) {
		emit_signal("sources_changed");
	}
}

PackedStringArray OMTDiscovery::get_sources() const {
	return sources;
}

bool OMTDiscovery::_contains(const PackedStringArray &p_array, const String &p_value) {
	for (int64_t i = 0; i < p_array.size(); ++i) {
		if (p_array[i] == p_value) {
			return true;
		}
	}
	return false;
}
