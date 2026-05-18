extends Control

@onready var _back_button: Button = $Layout/Header/BackButton
@onready var _refresh_button: Button = $Layout/Header/RefreshButton
@onready var _sources_list: ItemList = $Layout/Body/SourcesPanel/Margin/SourcesList
@onready var _event_log: RichTextLabel = $Layout/Body/EventPanel/Margin/EventLog
@onready var _status_label: Label = $Layout/StatusLabel
@onready var _discovery: OMTDiscovery = $OMTDiscovery

var _refresh_timer := 0.0


func _ready() -> void:
	_back_button.pressed.connect(_go_back)
	_refresh_button.pressed.connect(_refresh_sources)
	_discovery.source_added.connect(_on_source_added)
	_discovery.source_removed.connect(_on_source_removed)
	_discovery.sources_changed.connect(_on_sources_changed)
	_refresh_sources()


func _process(delta: float) -> void:
	_refresh_timer += delta
	if _refresh_timer < 3.0:
		return
	_refresh_timer = 0.0
	_refresh_sources()


func _refresh_sources() -> void:
	_discovery.refresh()
	_render_sources()


func _render_sources() -> void:
	var sources := _discovery.get_sources()
	_sources_list.clear()
	for source in sources:
		_sources_list.add_item(source)

	_status_label.text = "Discovered %d source(s). Refreshes every 3 seconds." % sources.size()


func _on_source_added(source: String) -> void:
	_append_event("added", source)


func _on_source_removed(source: String) -> void:
	_append_event("removed", source)


func _on_sources_changed() -> void:
	_render_sources()


func _append_event(action: String, source: String) -> void:
	_event_log.append_text("[%s] source %s: %s\n" % [Time.get_time_string_from_system(), action, source])


func _go_back() -> void:
	get_tree().change_scene_to_file("res://scenes/omt_demo_menu.tscn")
