extends Control

@onready var _back_button: Button = $Layout/Header/BackButton
@onready var _start_button: Button = $Layout/Header/StartButton
@onready var _stop_button: Button = $Layout/Header/StopButton
@onready var _preview: TextureRect = $Layout/PreviewPanel/Margin/Preview
@onready var _status_label: Label = $Layout/StatusLabel
@onready var _receiver: OMTReceiver = $OMTReceiver

var _status_timer := 0.0


func _ready() -> void:
	_back_button.pressed.connect(_go_back)
	_start_button.pressed.connect(_start_receiver)
	_stop_button.pressed.connect(_stop_receiver)
	_receiver.frame_ready.connect(_on_frame_ready)
	_receiver.connection_state_changed.connect(_on_connection_state_changed)

	_receiver.use_test_pattern = true
	_receiver.preview_mode = true
	_start_receiver()


func _process(delta: float) -> void:
	_status_timer += delta
	if _status_timer >= 0.5:
		_status_timer = 0.0
		_update_status()


func _start_receiver() -> void:
	if _receiver.get_connection_state() != OMTReceiver.STATE_CONNECTED:
		_receiver.start()
	_update_preview()
	_update_status()


func _stop_receiver() -> void:
	_receiver.stop()
	_update_status()


func _on_frame_ready() -> void:
	_update_preview()


func _on_connection_state_changed(_state: int) -> void:
	_update_status()


func _update_preview() -> void:
	var texture := _receiver.get_texture()
	if texture:
		_preview.texture = texture


func _update_status() -> void:
	var display_stats := _receiver.get_display_statistics()
	_status_label.text = (
		"Receiver state: %d\nframes_applied: %d\ntexture_size: %dx%d\nlast_error: %s"
		% [
			_receiver.get_connection_state(),
			display_stats.get("frames_applied", 0),
			display_stats.get("texture_width", 0),
			display_stats.get("texture_height", 0),
			_value_or_placeholder(_receiver.get_last_error()),
		]
	)


func _value_or_placeholder(value: String) -> String:
	return value if !value.is_empty() else "<none>"


func _go_back() -> void:
	_stop_receiver()
	get_tree().change_scene_to_file("res://scenes/omt_demo_menu.tscn")
