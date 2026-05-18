extends Control

const DEMOS := [
	{
		"title": "Runtime Status",
		"path": "res://scenes/omt_runtime_status_demo.tscn",
		"description": "Checks whether the native OMT runtime is available and shows the status helpers."
	},
	{
		"title": "Source Discovery",
		"path": "res://scenes/omt_discovery_demo.tscn",
		"description": "Uses OMTDiscovery to refresh network sources and react to source change signals."
	},
	{
		"title": "Receiver Texture",
		"path": "res://scenes/omt_receiver_texture_demo.tscn",
		"description": "Starts an OMTReceiver in test-pattern mode and displays its Texture2D."
	},
	{
		"title": "Viewport Output",
		"path": "res://scenes/omt_output_demo.tscn",
		"description": "Publishes an animated SubViewport with OMTOutput and shows output statistics."
	},
	{
		"title": "Clean Loopback",
		"path": "res://scenes/omt_loopback_clean.tscn",
		"description": "Connects OMTOutput back into OMTReceiver for the best end-to-end smoke test."
	},
	{
		"title": "VideoStreamPlayer",
		"path": "res://scenes/omt_video_stream_demo.tscn",
		"description": "Creates an OMTVideoStream resource and assigns it to Godot's VideoStreamPlayer API."
	},
]

@onready var _button_list: VBoxContainer = $Layout/Body/ButtonPanel/ButtonMargin/ButtonList
@onready var _description_label: Label = $Layout/Body/DescriptionPanel/DescriptionMargin/Description


func _ready() -> void:
	for demo in DEMOS:
		var button := Button.new()
		button.text = demo["title"]
		button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		button.tooltip_text = demo["description"]
		button.pressed.connect(_open_demo.bind(demo["path"]))
		button.mouse_entered.connect(_show_description.bind(demo["description"]))
		_button_list.add_child(button)

	_show_description("Choose a demo scene to see one part of the Godot OMT API in isolation.")


func _open_demo(path: String) -> void:
	get_tree().change_scene_to_file(path)


func _show_description(description: String) -> void:
	_description_label.text = description
