extends Control

const OMTTools = preload("res://tools/common/omt_tools.gd")

const TONE_CHUNK_FRAMES := 1024

@onready var _source_name_field: LineEdit = $Layout/Header/SourceNameField
@onready var _pattern_picker: OptionButton = $Layout/Controls/PatternPicker
@onready var _resolution_picker: OptionButton = $Layout/Controls/ResolutionPicker
@onready var _fps_spin: SpinBox = $Layout/Controls/FpsSpin
@onready var _quality_spin: SpinBox = $Layout/Controls/QualitySpin
@onready var _tone_enabled: CheckBox = $Layout/Controls/ToneEnabled
@onready var _tone_frequency: SpinBox = $Layout/Controls/ToneFrequency
@onready var _start_button: Button = $Layout/Header/StartButton
@onready var _stop_button: Button = $Layout/Header/StopButton
@onready var _metadata_button: Button = $Layout/Header/MetadataButton
@onready var _preview: TextureRect = $Layout/PreviewPanel/Margin/Preview
@onready var _status_label: Label = $Layout/StatusLabel
@onready var _runtime_label: Label = $Layout/RuntimeLabel
@onready var _subviewport: SubViewport = $SubViewport
@onready var _pattern_canvas: Control = $SubViewport/PatternCanvas
@onready var _output: OMTOutput = $OMTOutput
@onready var _tone_player: AudioStreamPlayer = $TonePlayer

var _tone_phase := 0.0
var _status_timer := 0.0
var _tone_playback: AudioStreamGeneratorPlayback


func _ready() -> void:
	_start_button.pressed.connect(_start_output)
	_stop_button.pressed.connect(_stop_output)
	_metadata_button.pressed.connect(_send_metadata)
	_pattern_picker.item_selected.connect(func(_index: int) -> void: _apply_pattern())
	_resolution_picker.item_selected.connect(func(_index: int) -> void: _apply_resolution())

	_setup_options()
	_setup_tone_player()
	_subviewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_preview.texture = _subviewport.get_texture()
	_runtime_label.text = OMTTools.runtime_status_text()
	_apply_resolution()
	_apply_pattern()
	_update_status()


func _process(delta: float) -> void:
	if _output.is_running():
		_sync_tone_playback()
		if _tone_enabled.button_pressed:
			_send_tone()
	_status_timer += delta
	if _status_timer >= 0.5:
		_status_timer = 0.0
		_update_status()


func _setup_options() -> void:
	for pattern in ["SMPTE Bars", "Checkerboard", "Grid"]:
		_pattern_picker.add_item(pattern)
	for item in ["1280x720", "1920x1080", "640x360"]:
		_resolution_picker.add_item(item)
	_resolution_picker.select(0)
	_fps_spin.value = 30
	_quality_spin.value = 90
	_tone_frequency.value = 1000


func _setup_tone_player() -> void:
	var generator := AudioStreamGenerator.new()
	generator.mix_rate = AudioServer.get_mix_rate()
	generator.buffer_length = 0.25
	_tone_player.stream = generator


func _apply_pattern() -> void:
	_pattern_canvas.set("pattern_name", _pattern_picker.get_item_text(_pattern_picker.selected))
	_pattern_canvas.queue_redraw()


func _apply_resolution() -> void:
	var parts := _resolution_picker.get_item_text(_resolution_picker.selected).split("x")
	if parts.size() != 2:
		return
	_subviewport.size = Vector2i(int(parts[0]), int(parts[1]))
	_pattern_canvas.size = Vector2(_subviewport.size)


func _start_output() -> void:
	_apply_resolution()
	_apply_pattern()
	_tone_phase = 0.0
	_output.source_name = _source_name_field.text.strip_edges()
	if _output.source_name.is_empty():
		_output.source_name = "Godot OMT Test Pattern"
	_output.viewport_path = NodePath("../SubViewport")
	_output.frame_rate = int(_fps_spin.value)
	_output.quality = int(_quality_spin.value)
	_output.metadata = _metadata_text()
	_output.enabled = true
	_output.start()
	if _tone_enabled.button_pressed:
		_tone_player.play()
		_tone_playback = _tone_player.get_stream_playback()
	_update_status()


func _sync_tone_playback() -> void:
	if _tone_enabled.button_pressed:
		if not _tone_player.playing:
			_tone_player.play()
			_tone_playback = _tone_player.get_stream_playback()
	elif _tone_player.playing:
		_tone_player.stop()
		_tone_playback = null


func _stop_output() -> void:
	_tone_player.stop()
	_tone_playback = null
	_output.enabled = false
	_output.stop()
	_update_status()


func _send_metadata() -> void:
	_output.send_metadata(_metadata_text())
	_update_status()


func _send_tone() -> void:
	var sample_rate := int(AudioServer.get_mix_rate())
	var frequency := float(_tone_frequency.value)
	var frames := TONE_CHUNK_FRAMES
	if _tone_playback:
		frames = min(frames, _tone_playback.get_frames_available())
	if frames <= 0:
		return
	var samples := PackedFloat32Array()
	samples.resize(frames * 2)
	for frame in range(frames):
		var value := sin(_tone_phase) * 0.2
		samples[frame * 2] = value
		samples[frame * 2 + 1] = value
		if _tone_playback:
			_tone_playback.push_frame(Vector2(value, value))
		_tone_phase += TAU * frequency / sample_rate
		if _tone_phase >= TAU:
			_tone_phase -= TAU
	_output.send_audio_frame(samples, sample_rate, 2)


func _metadata_text() -> String:
	return "OMT Test Pattern | pattern=%s | resolution=%s | fps=%d | tone=%sHz" % [
		_pattern_picker.get_item_text(_pattern_picker.selected),
		_resolution_picker.get_item_text(_resolution_picker.selected),
		int(_fps_spin.value),
		int(_tone_frequency.value),
	]


func _update_status() -> void:
	_status_label.text = (
		"running: %s | address: %s | video frames: %d | audio frames: %d | error: %s"
		% [
			_output.is_running(),
			OMTTools.value_or_placeholder(_output.get_address()),
			_output.get_frames_sent(),
			_output.get_audio_frames_sent(),
			OMTTools.value_or_placeholder(_output.get_last_error()),
		]
	)
