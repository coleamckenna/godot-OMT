extends Control

const OMTCaptureDevices = preload("res://tools/common/omt_capture_devices.gd")
const OMTTools = preload("res://tools/common/omt_tools.gd")

const AUDIO_BUS_NAME := "OMT Mic Capture"
const AUDIO_CHUNK_FRAMES := 1024

@onready var _source_name_field: LineEdit = $Layout/Header/SourceNameField
@onready var _camera_picker: OptionButton = $Layout/Controls/CameraPicker
@onready var _mic_picker: OptionButton = $Layout/Controls/MicPicker
@onready var _resolution_picker: OptionButton = $Layout/Controls/ResolutionPicker
@onready var _fps_spin: SpinBox = $Layout/Controls/FpsSpin
@onready var _quality_spin: SpinBox = $Layout/Controls/QualitySpin
@onready var _audio_enabled: CheckBox = $Layout/Controls/AudioEnabled
@onready var _mute_check: CheckBox = $Layout/Controls/MuteCheck
@onready var _refresh_button: Button = $Layout/Controls/RefreshDevicesButton
@onready var _start_button: Button = $Layout/Header/StartButton
@onready var _stop_button: Button = $Layout/Header/StopButton
@onready var _preview: TextureRect = $Layout/PreviewPanel/Margin/Preview
@onready var _level_bar: ProgressBar = $Layout/Status/LevelBar
@onready var _status_label: Label = $Layout/Status/StatusLabel
@onready var _runtime_label: Label = $Layout/RuntimeLabel
@onready var _subviewport: SubViewport = $SubViewport
@onready var _camera_rect: TextureRect = $SubViewport/CameraFrame
@onready var _fallback_label: Label = $SubViewport/FallbackLabel
@onready var _output: OMTOutput = $OMTOutput
@onready var _mic_player: AudioStreamPlayer = $MicPlayer

var _capture_effect: AudioEffectCapture
var _capture_bus_index := -1
var _status_timer := 0.0


func _ready() -> void:
	_start_button.pressed.connect(_start_output)
	_stop_button.pressed.connect(_stop_output)
	_refresh_button.pressed.connect(_refresh_devices)
	_camera_picker.item_selected.connect(func(_index: int) -> void: _apply_camera_selection())
	_resolution_picker.item_selected.connect(func(_index: int) -> void: _apply_resolution_selection())

	_subviewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_preview.texture = _subviewport.get_texture()
	_runtime_label.text = OMTTools.runtime_status_text()
	_setup_audio_capture_bus()
	_setup_resolution_options()
	_refresh_devices()
	_apply_resolution_selection()
	_update_status()


func _process(delta: float) -> void:
	if _output.is_running() and _audio_enabled.button_pressed and not _mute_check.button_pressed:
		_send_available_audio()

	_status_timer += delta
	if _status_timer >= 0.5:
		_status_timer = 0.0
		_update_status()


func _exit_tree() -> void:
	_stop_output()
	if _capture_bus_index >= 0 and _capture_bus_index < AudioServer.get_bus_count():
		AudioServer.remove_bus(_capture_bus_index)


func _setup_resolution_options() -> void:
	for item in ["1280x720", "1920x1080", "640x360"]:
		_resolution_picker.add_item(item)
	_resolution_picker.select(0)
	_fps_spin.value = 30
	_quality_spin.value = 50


func _refresh_devices() -> void:
	_camera_picker.clear()
	for device in OMTCaptureDevices.get_camera_devices():
		_camera_picker.add_item(str(device["name"]))
		_camera_picker.set_item_metadata(_camera_picker.item_count - 1, device)
	if _camera_picker.item_count == 0:
		_camera_picker.add_item("No camera feeds found")
		_camera_picker.set_item_disabled(0, true)
	else:
		_camera_picker.select(0)

	_mic_picker.clear()
	for device_name in OMTCaptureDevices.get_microphone_devices():
		_mic_picker.add_item(device_name)
		_mic_picker.set_item_metadata(_mic_picker.item_count - 1, device_name)
	if _mic_picker.item_count == 0:
		_mic_picker.add_item("Default input")
		_mic_picker.set_item_metadata(0, "")

	_apply_camera_selection()
	_update_status()


func _apply_camera_selection() -> void:
	if _camera_picker.item_count == 0 or _camera_picker.is_item_disabled(_camera_picker.selected):
		_camera_rect.texture = null
		_fallback_label.visible = true
		return

	var device: Dictionary = _camera_picker.get_item_metadata(_camera_picker.selected)
	var feed = device.get("feed")
	if feed != null and feed.has_method("set_active"):
		feed.call("set_active", true)

	var texture := OMTCaptureDevices.create_camera_texture(int(device["id"]))
	_camera_rect.texture = texture
	_fallback_label.visible = texture == null


func _apply_resolution_selection() -> void:
	var parts := _resolution_picker.get_item_text(_resolution_picker.selected).split("x")
	if parts.size() != 2:
		return
	_subviewport.size = Vector2i(int(parts[0]), int(parts[1]))
	_camera_rect.size = Vector2(_subviewport.size)
	_fallback_label.size = Vector2(_subviewport.size)


func _setup_audio_capture_bus() -> void:
	_capture_bus_index = AudioServer.get_bus_count()
	AudioServer.add_bus(_capture_bus_index)
	AudioServer.set_bus_name(_capture_bus_index, AUDIO_BUS_NAME)
	AudioServer.set_bus_mute(_capture_bus_index, true)
	_capture_effect = AudioEffectCapture.new()
	AudioServer.add_bus_effect(_capture_bus_index, _capture_effect, 0)
	_mic_player.stream = AudioStreamMicrophone.new()
	_mic_player.bus = AUDIO_BUS_NAME


func _start_output() -> void:
	if _mic_picker.item_count > 0:
		var input_device := str(_mic_picker.get_item_metadata(_mic_picker.selected))
		if not input_device.is_empty():
			AudioServer.input_device = input_device

	_apply_resolution_selection()
	_apply_camera_selection()
	_capture_effect.clear_buffer()
	if _audio_enabled.button_pressed:
		_mic_player.play()

	_output.source_name = _source_name_field.text.strip_edges()
	if _output.source_name.is_empty():
		_output.source_name = "Godot OMT Camera Mic"
	_output.viewport_path = NodePath("../SubViewport")
	_output.frame_rate = int(_fps_spin.value)
	_output.quality = int(_quality_spin.value)
	_output.metadata = "Created by OMT Camera And Microphone"
	_output.enabled = true
	_output.start()
	_update_status()


func _stop_output() -> void:
	_mic_player.stop()
	_output.enabled = false
	_output.stop()
	_update_status()


func _send_available_audio() -> void:
	if _capture_effect == null:
		return
	var available: int = _capture_effect.get_frames_available()
	if available <= 0:
		_level_bar.value = 0.0
		return
	var frames: int = min(available, AUDIO_CHUNK_FRAMES)
	var captured: PackedVector2Array = _capture_effect.get_buffer(frames)
	if captured.is_empty():
		return

	var samples := PackedFloat32Array()
	samples.resize(captured.size() * 2)
	var write_index := 0
	for frame in captured:
		samples[write_index] = frame.x
		samples[write_index + 1] = frame.y
		write_index += 2
	_level_bar.value = OMTTools.mono_peak(samples) * 100.0
	_output.send_audio_frame(samples, int(AudioServer.get_mix_rate()), 2)


func _update_status() -> void:
	_status_label.text = (
		"running: %s | address: %s | video frames: %d | audio frames: %d | audio samples: %d | error: %s"
		% [
			_output.is_running(),
			OMTTools.value_or_placeholder(_output.get_address()),
			_output.get_frames_sent(),
			_output.get_audio_frames_sent(),
			_output.get_audio_samples_sent(),
			OMTTools.value_or_placeholder(_output.get_last_error()),
		]
	)
