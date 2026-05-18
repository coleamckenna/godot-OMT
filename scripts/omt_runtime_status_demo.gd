extends Control

@onready var _back_button: Button = $Layout/Header/BackButton
@onready var _refresh_button: Button = $Layout/Header/RefreshButton
@onready var _status_label: Label = $Layout/StatusPanel/Margin/StatusLabel


func _ready() -> void:
	_back_button.pressed.connect(_go_back)
	_refresh_button.pressed.connect(_update_status)
	_update_status()


func _update_status() -> void:
	var runtime_status: Dictionary = OMT.get_runtime_status()
	var lines := [
		"OMT.is_available(): %s" % OMT.is_available(),
		"OMT.is_stub_build(): %s" % OMT.is_stub_build(),
		"OMT.get_version(): %s" % OMT.get_version(),
		"OMT.get_runtime_error(): %s" % _value_or_placeholder(OMT.get_runtime_error()),
		"",
		"Runtime status dictionary:"
	]

	for key in runtime_status.keys():
		lines.append("  %s = %s" % [key, runtime_status[key]])

	_status_label.text = "\n".join(lines)


func _value_or_placeholder(value: String) -> String:
	return value if !value.is_empty() else "<none>"


func _go_back() -> void:
	get_tree().change_scene_to_file("res://scenes/omt_demo_menu.tscn")
