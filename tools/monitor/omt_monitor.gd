extends Control

const OMTTools = preload("res://tools/common/omt_tools.gd")

@onready var _refresh_button: Button = $Layout/Header/RefreshButton
@onready var _source_picker: OptionButton = $Layout/Header/SourcePicker
@onready var _address_field: LineEdit = $Layout/Header/AddressField
@onready var _connect_button: Button = $Layout/Header/ConnectButton
@onready var _disconnect_button: Button = $Layout/Header/DisconnectButton
@onready var _fullscreen_button: Button = $Layout/Header/FullscreenButton
@onready var _preview: TextureRect = $Layout/Body/PreviewPanel/Margin/Preview
@onready var _stats: RichTextLabel = $Layout/Body/StatsPanel/Margin/Stats
@onready var _status_label: Label = $Layout/StatusLabel
@onready var _runtime_label: Label = $Layout/RuntimeLabel
@onready var _discovery: OMTDiscovery = $OMTDiscovery
@onready var _receiver: OMTReceiver = $OMTReceiver

var _refresh_timer := 0.0
var _status_timer := 0.0


func _ready() -> void:
	_refresh_button.pressed.connect(_refresh_sources)
	_source_picker.item_selected.connect(_on_source_selected)
	_connect_button.pressed.connect(_connect_source)
	_disconnect_button.pressed.connect(_disconnect_source)
	_fullscreen_button.pressed.connect(_toggle_fullscreen)
	_discovery.sources_changed.connect(_render_sources)
	_discovery.source_added.connect(func(_source: String) -> void: _render_sources())
	_discovery.source_removed.connect(func(_source: String) -> void: _render_sources())
	_receiver.frame_ready.connect(_update_preview)
	_receiver.connection_state_changed.connect(func(_state: int) -> void: _update_status())
	_receiver.metadata_received.connect(func(_metadata: String) -> void: _update_status())
	_receiver.audio_frame_received.connect(func(_audio_info: Dictionary) -> void: _update_status())

	_receiver.auto_start = false
	_receiver.preview_mode = true
	_runtime_label.text = OMTTools.runtime_status_text()
	_refresh_sources()
	_update_status()


func _process(delta: float) -> void:
	_refresh_timer += delta
	if _refresh_timer >= 3.0:
		_refresh_timer = 0.0
		_refresh_sources()

	_status_timer += delta
	if _status_timer >= 0.5:
		_status_timer = 0.0
		_update_status()


func _refresh_sources() -> void:
	_discovery.refresh()
	_render_sources()


func _render_sources() -> void:
	var current := _address_field.text
	_source_picker.clear()
	for source in _discovery.get_sources():
		_source_picker.add_item(source)
		_source_picker.set_item_metadata(_source_picker.item_count - 1, source)
	if _source_picker.item_count == 0:
		_source_picker.add_item("No OMT sources found")
		_source_picker.set_item_disabled(0, true)
	elif current.is_empty():
		_source_picker.select(0)
		_address_field.text = str(_source_picker.get_item_metadata(0))
	_update_status()


func _on_source_selected(index: int) -> void:
	if index < 0 or index >= _source_picker.item_count or _source_picker.is_item_disabled(index):
		return
	_address_field.text = str(_source_picker.get_item_metadata(index))


func _connect_source() -> void:
	var source_address := _address_field.text.strip_edges()
	if source_address.is_empty():
		_status_label.text = "Enter or select an OMT source address before connecting."
		return
	_receiver.stop()
	_receiver.source_address = source_address
	_receiver.use_test_pattern = false
	_receiver.preview_mode = true
	_receiver.start()
	_update_preview()
	_update_status()


func _disconnect_source() -> void:
	_receiver.stop()
	_update_status()


func _update_preview() -> void:
	var texture := _receiver.get_texture()
	if texture:
		_preview.texture = texture


func _update_status() -> void:
	var state := _state_name(_receiver.get_connection_state())
	var display_stats := _receiver.get_display_statistics()
	var video_stats := _receiver.get_video_statistics()
	var audio_stats := _receiver.get_audio_statistics()
	var sender_info := _receiver.get_sender_information()
	var audio_info := _receiver.get_last_audio_info()

	_status_label.text = "State: %s | Source: %s | Error: %s" % [
		state,
		OMTTools.value_or_placeholder(_receiver.source_address),
		OMTTools.value_or_placeholder(_receiver.get_last_error()),
	]
	_stats.text = "\n\n".join([
		OMTTools.dictionary_to_lines("Display", display_stats),
		OMTTools.dictionary_to_lines("Video", video_stats),
		OMTTools.dictionary_to_lines("Audio", audio_stats),
		OMTTools.dictionary_to_lines("Last Audio Frame", audio_info),
		OMTTools.dictionary_to_lines("Sender", sender_info),
		"Metadata\n  %s" % OMTTools.value_or_placeholder(_receiver.get_last_metadata()),
	])


func _toggle_fullscreen() -> void:
	var mode := DisplayServer.window_get_mode()
	if mode == DisplayServer.WINDOW_MODE_FULLSCREEN:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_WINDOWED)
		_fullscreen_button.text = "Fullscreen"
	else:
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_FULLSCREEN)
		_fullscreen_button.text = "Windowed"


func _state_name(state: int) -> String:
	match state:
		OMTReceiver.STATE_DISCONNECTED:
			return "disconnected"
		OMTReceiver.STATE_CONNECTING:
			return "connecting"
		OMTReceiver.STATE_CONNECTED:
			return "connected"
		OMTReceiver.STATE_ERROR:
			return "error"
		_:
			return str(state)
