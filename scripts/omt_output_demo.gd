extends Control

@onready var _back_button: Button = $Layout/Header/BackButton
@onready var _start_button: Button = $Layout/Header/StartButton
@onready var _stop_button: Button = $Layout/Header/StopButton
@onready var _metadata_button: Button = $Layout/Header/MetadataButton
@onready var _preview: TextureRect = $Layout/PreviewPanel/Margin/Preview
@onready var _status_label: Label = $Layout/StatusLabel
@onready var _subviewport: SubViewport = $SubViewport
@onready var _background: ColorRect = $SubViewport/SourceBackground
@onready var _sprite: Sprite2D = $SubViewport/SourceSprite
@onready var _output: OMTOutput = $OMTOutput

var _time := 0.0
var _status_timer := 0.0


func _ready() -> void:
	_back_button.pressed.connect(_go_back)
	_start_button.pressed.connect(_start_output)
	_stop_button.pressed.connect(_stop_output)
	_metadata_button.pressed.connect(_send_metadata)
	_output.started.connect(_update_status)
	_output.stopped.connect(_update_status)
	_output.error.connect(_on_output_error)

	_subviewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_preview.texture = _subviewport.get_texture()
	_start_output()


func _process(delta: float) -> void:
	_time += delta
	_animate_source()

	_status_timer += delta
	if _status_timer >= 0.5:
		_status_timer = 0.0
		_update_status()


func _start_output() -> void:
	_output.source_name = "Godot OMT Output Demo %d" % Time.get_ticks_msec()
	_output.viewport_path = NodePath("../SubViewport")
	_output.frame_rate = 30
	_output.quality = 50
	_output.metadata = "Created by the Godot OMT output demo"
	_output.enabled = true
	_output.start()
	_update_status()


func _stop_output() -> void:
	_output.enabled = false
	_output.stop()
	_update_status()


func _send_metadata() -> void:
	_output.send_metadata("Manual metadata at %s" % Time.get_time_string_from_system())
	_update_status()


func _animate_source() -> void:
	_background.color = Color(
		0.5 + sin(_time * 1.2) * 0.5,
		0.5 + sin(_time * 1.7 + 1.0) * 0.5,
		0.5 + sin(_time * 2.1 + 2.0) * 0.5,
		1.0
	)
	_sprite.position = Vector2(
		256.0 + sin(_time * 2.0) * 190.0,
		256.0 + cos(_time * 1.4) * 190.0
	)
	_sprite.rotation = _time
	_sprite.scale = Vector2.ONE * (1.0 + sin(_time * 3.0) * 0.2)


func _update_status() -> void:
	var router_status := _output.get_router_status()
	_status_label.text = (
		"running: %s\naddress: %s\nframes_attempted: %d\nframes_sent: %d\nlast_send_result: %d\nlast_checksum: %d\nlast_error: %s\nrouter: %s"
		% [
			_output.is_running(),
			_value_or_placeholder(_output.get_address()),
			_output.get_frames_attempted(),
			_output.get_frames_sent(),
			_output.get_last_send_result(),
			_output.get_last_frame_checksum(),
			_value_or_placeholder(_output.get_last_error()),
			str(router_status),
		]
	)


func _on_output_error(_message: String) -> void:
	_update_status()


func _value_or_placeholder(value: String) -> String:
	return value if !value.is_empty() else "<none>"


func _go_back() -> void:
	_stop_output()
	get_tree().change_scene_to_file("res://scenes/omt_demo_menu.tscn")
