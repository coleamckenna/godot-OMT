
extends Control

@onready var _omt: OMTReceiver = $OMTReceiver
@onready var _view: TextureRect = $TextureRect
@onready var _output: OMTOutput = $OMTOutput
@onready var _animation_player: AnimationPlayer = $SubViewport/AnimationPlayer
@onready var _sprite: Sprite2D = $SubViewport/Sprite2D
@onready var _background: ColorRect = $SubViewport/ColorRect

var _status_timer := 0.0
var _motion_time := 0.0


func _ready() -> void:
	_omt.frame_ready.connect(_on_frame_ready)
	_animation_player.play("new_animation")
	_on_frame_ready()
	print(OMTReceiver.discover_sources())


func _process(delta: float) -> void:
	_motion_time += delta
	_sprite.position = Vector2(
		256.0 + sin(_motion_time * 2.0) * 180.0,
		256.0 + cos(_motion_time * 1.5) * 180.0
	)
	_background.color = Color(
		0.5 + sin(_motion_time * 1.1) * 0.5,
		0.5 + sin(_motion_time * 1.7) * 0.5,
		0.5 + sin(_motion_time * 2.3) * 0.5,
		1.0
	)

	_status_timer += delta
	if _status_timer < 1.0:
		return
	_status_timer = 0.0

	var video_stats := _omt.get_video_statistics()
	var display_stats := _omt.get_display_statistics()
	var router_status := _output.get_router_status()
	print(
		"OMT loopback status: attempted=",
		_output.get_frames_attempted(),
		" accepted=",
		_output.get_frames_sent(),
		" send_result=",
		_output.get_last_send_result(),
		" checksum=",
		_output.get_last_frame_checksum(),
		" received=",
		video_stats.get("frames", 0),
		" dropped=",
		video_stats.get("frames_dropped", 0),
		" receiver_state=",
		_omt.get_connection_state(),
		" applied=",
		display_stats.get("frames_applied", 0),
		" applied_checksum=",
		display_stats.get("last_applied_checksum", 0),
		" texture_size=",
		Vector2i(display_stats.get("texture_width", 0), display_stats.get("texture_height", 0)),
		" output_error=",
		_output.get_last_error(),
		" router=",
		router_status
	)


func _on_frame_ready() -> void:
	var tex := _omt.get_texture()
	if tex:
		_view.texture = tex
