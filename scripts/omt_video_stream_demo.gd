extends Control

@onready var _back_button: Button = $Layout/Header/BackButton
@onready var _play_button: Button = $Layout/Header/PlayButton
@onready var _stop_button: Button = $Layout/Header/StopButton
@onready var _player: VideoStreamPlayer = $Layout/PlayerPanel/Margin/VideoStreamPlayer
@onready var _status_label: Label = $Layout/StatusLabel

var _stream: OMTVideoStream


func _ready() -> void:
	_back_button.pressed.connect(_go_back)
	_play_button.pressed.connect(_play)
	_stop_button.pressed.connect(_stop)

	_stream = OMTVideoStream.new()
	_stream.source_address = ""
	_stream.preview_mode = true
	_stream.preferred_format = 2
	_player.stream = _stream
	_play()


func _process(_delta: float) -> void:
	_update_status()


func _play() -> void:
	_player.play()
	_update_status()


func _stop() -> void:
	_player.stop()
	_update_status()


func _update_status() -> void:
	_status_label.text = (
		"VideoStreamPlayer playing: %s\nOMTVideoStream.source_address: %s\npreview_mode: %s\npreferred_format: %d\n\nThis scene demonstrates the resource/playback path. The current playback implementation is minimal, so use the receiver and loopback demos for full frame transport validation."
		% [
			_player.is_playing(),
			_value_or_placeholder(_stream.source_address),
			_stream.preview_mode,
			_stream.preferred_format,
		]
	)


func _value_or_placeholder(value: String) -> String:
	return value if !value.is_empty() else "<empty>"


func _go_back() -> void:
	_player.stop()
	get_tree().change_scene_to_file("res://scenes/omt_demo_menu.tscn")
