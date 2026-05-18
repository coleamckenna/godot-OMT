extends Control

@onready var _subviewport: SubViewport = $SubViewport
@onready var _source_background: ColorRect = $SubViewport/SourceBackground
@onready var _source_sprite: Sprite2D = $SubViewport/SourceSprite
@onready var _local_preview: TextureRect = $Layout/Previews/LocalColumn/LocalPreview
@onready var _received_preview: TextureRect = $Layout/Previews/ReceivedColumn/ReceivedPreview
@onready var _status_label: Label = $Layout/StatusLabel
@onready var _output: OMTOutput = $OMTOutput
@onready var _receiver: OMTReceiver = $OMTReceiver

var _time := 0.0
var _connect_attempted := false
var _status_timer := 0.0


func _ready() -> void:
	_subviewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_local_preview.texture = _subviewport.get_texture()
	_receiver.frame_ready.connect(_on_receiver_frame_ready)

	_output.source_name = "Godot OMT Loopback %d" % Time.get_ticks_msec()
	_output.viewport_path = NodePath("../SubViewport")
	_output.enabled = true

	_status_label.text = "Starting OMT output..."


func _process(delta: float) -> void:
	_time += delta
	_animate_source()
	_try_connect_receiver()
	_update_status(delta)

	# Keep this assignment explicit so the demo does not depend on signal timing.
	var received_texture := _receiver.get_texture()
	if received_texture:
		_received_preview.texture = received_texture


func _animate_source() -> void:
	_source_background.color = Color(
		0.5 + sin(_time * 1.3) * 0.5,
		0.5 + sin(_time * 1.9 + 1.0) * 0.5,
		0.5 + sin(_time * 2.7 + 2.0) * 0.5,
		1.0
	)
	_source_sprite.position = Vector2(
		256.0 + sin(_time * 2.4) * 190.0,
		256.0 + cos(_time * 1.7) * 190.0
	)
	_source_sprite.rotation = _time
	_source_sprite.scale = Vector2.ONE * (1.0 + sin(_time * 3.0) * 0.25)


func _try_connect_receiver() -> void:
	if _connect_attempted:
		return

	var address := _output.get_address()
	if address.is_empty():
		return

	_connect_attempted = true
	_receiver.source_address = address
	_receiver.start()
	_status_label.text = "Connected receiver to %s" % address


func _update_status(delta: float) -> void:
	_status_timer += delta
	if _status_timer < 1.0:
		return
	_status_timer = 0.0

	var video_stats := _receiver.get_video_statistics()
	var display_stats := _receiver.get_display_statistics()
	var router_status := _output.get_router_status()
	_status_label.text = (
		"source=%s\nattempted=%d accepted=%d send_checksum=%d\nreceived=%d applied=%d applied_checksum=%d texture=%dx%d\noutput_error=%s router=%s"
		% [
			_output.get_address(),
			_output.get_frames_attempted(),
			_output.get_frames_sent(),
			_output.get_last_frame_checksum(),
			video_stats.get("frames", 0),
			display_stats.get("frames_applied", 0),
			display_stats.get("last_applied_checksum", 0),
			display_stats.get("texture_width", 0),
			display_stats.get("texture_height", 0),
			_output.get_last_error(),
			str(router_status),
		]
	)


func _on_receiver_frame_ready() -> void:
	var received_texture := _receiver.get_texture()
	if received_texture:
		_received_preview.texture = received_texture
