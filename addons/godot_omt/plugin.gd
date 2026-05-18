@tool
extends EditorPlugin


var _inspector_plugin: EditorInspectorPlugin


func _enter_tree() -> void:
	_inspector_plugin = OMTReceiverInspectorPlugin.new()
	add_inspector_plugin(_inspector_plugin)
	add_tool_menu_item("OMT Runtime Status", _print_runtime_status)


func _exit_tree() -> void:
	if _inspector_plugin:
		remove_inspector_plugin(_inspector_plugin)
		_inspector_plugin = null
	remove_tool_menu_item("OMT Runtime Status")


func _print_runtime_status() -> void:
	if ClassDB.class_exists("OMT"):
		print("OMT runtime status: ", ClassDB.class_call_static("OMT", "get_runtime_status"))
	else:
		print("OMT runtime status: OMT class is not registered. Check that the GDExtension loaded.")


class OMTReceiverInspectorPlugin:
	extends EditorInspectorPlugin

	func _can_handle(object: Object) -> bool:
		return _is_omt_receiver(object)

	func _parse_property(object, type, name, hint_type, hint_string, usage_flags, wide) -> bool:
		if name != "source_address":
			return false

		add_property_editor(name, OMTSourceAddressProperty.new())
		return true

	func _is_omt_receiver(object: Object) -> bool:
		return (
			object != null
			and object.has_method("get_source_address")
			and object.has_method("set_source_address")
		)


class OMTSourceAddressProperty:
	extends EditorProperty

	var _container := HBoxContainer.new()
	var _dropdown := OptionButton.new()
	var _refresh_button := Button.new()
	var _manual_value := ""
	var _updating := false


	func _init() -> void:
		label = "Source Address"

		_dropdown.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		_dropdown.item_selected.connect(_on_item_selected)

		_refresh_button.text = "Refresh"
		_refresh_button.tooltip_text = "Refresh discovered OMT sources"
		_refresh_button.pressed.connect(_refresh_sources)

		_container.add_child(_dropdown)
		_container.add_child(_refresh_button)
		add_child(_container)
		add_focusable(_dropdown)


	func _update_property() -> void:
		if _updating:
			return

		var edited := get_edited_object()
		if edited == null:
			return

		_manual_value = str(edited.get("source_address"))
		_populate_dropdown(_manual_value)


	func _populate_dropdown(current_value: String) -> void:
		_updating = true
		_dropdown.clear()

		var sources := _discover_sources()
		var selected_index := -1

		if current_value != "":
			_dropdown.add_item(current_value)
			_dropdown.set_item_metadata(0, current_value)
			selected_index = 0

		for source in sources:
			var source_text := str(source)
			if source_text == "":
				continue
			if source_text == current_value:
				continue

			var idx := _dropdown.item_count
			_dropdown.add_item(source_text)
			_dropdown.set_item_metadata(idx, source_text)

			if selected_index == -1 and current_value == source_text:
				selected_index = idx

		if _dropdown.item_count == 0:
			_dropdown.add_item("No OMT sources found")
			_dropdown.set_item_disabled(0, true)
			_dropdown.tooltip_text = "No OMT sources found. Start an OMT sender, then click Refresh."
		else:
			_dropdown.tooltip_text = ""
			_dropdown.select(max(selected_index, 0))

		_updating = false


	func _discover_sources() -> PackedStringArray:
		if not ClassDB.class_exists("OMTReceiver"):
			return PackedStringArray()

		var result = ClassDB.class_call_static("OMTReceiver", "discover_sources")
		if result is PackedStringArray:
			return result

		var converted := PackedStringArray()
		if result is Array:
			for item in result:
				converted.push_back(str(item))
		return converted


	func _refresh_sources() -> void:
		var edited := get_edited_object()
		if edited != null and edited.has_method("refresh_sources"):
			edited.refresh_sources()
		_update_property()


	func _on_item_selected(index: int) -> void:
		if _updating:
			return
		if index < 0 or index >= _dropdown.item_count:
			return
		if _dropdown.is_item_disabled(index):
			return

		var value := str(_dropdown.get_item_metadata(index))
		emit_changed("source_address", value)
