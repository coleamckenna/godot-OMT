class_name OMTTools
extends RefCounted


static func runtime_status_text() -> String:
	if not ClassDB.class_exists("OMT"):
		return "OMT runtime unavailable: GDExtension class is not registered."
	var status = ClassDB.class_call_static("OMT", "get_runtime_status")
	return "OMT runtime: %s" % str(status)


static func value_or_placeholder(value: Variant, placeholder := "<none>") -> String:
	var text := str(value)
	return text if not text.is_empty() else placeholder


static func dictionary_to_lines(title: String, values: Dictionary) -> String:
	var lines: Array[String] = [title]
	if values.is_empty():
		lines.append("  <no data>")
		return "\n".join(lines)
	for key in values.keys():
		lines.append("  %s: %s" % [str(key), str(values[key])])
	return "\n".join(lines)


static func mono_peak(samples: PackedFloat32Array) -> float:
	var peak := 0.0
	for sample in samples:
		peak = max(peak, abs(sample))
	return peak
