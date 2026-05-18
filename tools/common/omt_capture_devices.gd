class_name OMTCaptureDevices
extends RefCounted


static func get_camera_devices() -> Array[Dictionary]:
	var devices: Array[Dictionary] = []
	if not ClassDB.class_exists("CameraServer"):
		return devices

	CameraServer.set_monitoring_feeds(true)
	var count: int = CameraServer.get_feed_count()
	for index in range(count):
		var feed := CameraServer.get_feed(index)
		if feed == null:
			continue
		var name := "Camera %d" % index
		if feed.has_method("get_name"):
			name = str(feed.get_name())
		var feed_id := index
		if feed.has_method("get_id"):
			feed_id = int(feed.get_id())
		devices.append({
			"index": index,
			"id": feed_id,
			"name": name,
			"feed": feed,
		})
	return devices


static func get_microphone_devices() -> PackedStringArray:
	if not ClassDB.class_exists("AudioServer"):
		return PackedStringArray()
	return AudioServer.get_input_device_list()


static func create_camera_texture(feed_id: int) -> Texture2D:
	if not ClassDB.class_exists("CameraTexture"):
		return null
	var texture: Object = ClassDB.instantiate("CameraTexture")
	if texture == null:
		return null
	if texture.has_method("set_camera_feed_id"):
		texture.call("set_camera_feed_id", feed_id)
	else:
		texture.set("camera_feed_id", feed_id)
	if texture.has_method("set_camera_active"):
		texture.call("set_camera_active", true)
	else:
		texture.set("camera_is_active", true)
	return texture as Texture2D
