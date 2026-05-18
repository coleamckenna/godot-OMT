extends Control

var pattern_name := "SMPTE Bars"
var elapsed := 0.0
var frame_count := 0


func _process(delta: float) -> void:
	elapsed += delta
	frame_count += 1
	queue_redraw()


func _draw() -> void:
	match pattern_name:
		"Checkerboard":
			_draw_checkerboard()
		"Grid":
			_draw_grid()
		_:
			_draw_bars()
	_draw_motion_target()
	_draw_overlay()


func _draw_bars() -> void:
	var colors := [
		Color.WHITE,
		Color.YELLOW,
		Color.CYAN,
		Color.GREEN,
		Color.MAGENTA,
		Color.RED,
		Color.BLUE,
		Color.BLACK,
	]
	var bar_width := size.x / colors.size()
	for index in range(colors.size()):
		draw_rect(Rect2(index * bar_width, 0.0, bar_width + 1.0, size.y), colors[index])


func _draw_checkerboard() -> void:
	var cell := 64.0
	for y in range(int(ceil(size.y / cell))):
		for x in range(int(ceil(size.x / cell))):
			var light := ((x + y) % 2) == 0
			draw_rect(Rect2(x * cell, y * cell, cell, cell), Color(0.85, 0.85, 0.85) if light else Color(0.08, 0.08, 0.08))


func _draw_grid() -> void:
	draw_rect(Rect2(Vector2.ZERO, size), Color(0.02, 0.03, 0.04))
	for x in range(0, int(size.x), 80):
		draw_line(Vector2(x, 0), Vector2(x, size.y), Color(0.2, 0.7, 1.0), 2.0)
	for y in range(0, int(size.y), 80):
		draw_line(Vector2(0, y), Vector2(size.x, y), Color(0.2, 0.7, 1.0), 2.0)


func _draw_motion_target() -> void:
	var radius := 42.0
	var center := Vector2(
		size.x * 0.5 + sin(elapsed * 1.3) * size.x * 0.32,
		size.y * 0.5 + cos(elapsed * 1.7) * size.y * 0.28
	)
	draw_circle(center, radius, Color(1.0, 1.0, 1.0, 0.85))
	draw_arc(center, radius * 0.65, 0.0, TAU, 64, Color.BLACK, 5.0)
	draw_line(center + Vector2(-radius, 0), center + Vector2(radius, 0), Color.BLACK, 4.0)
	draw_line(center + Vector2(0, -radius), center + Vector2(0, radius), Color.BLACK, 4.0)


func _draw_overlay() -> void:
	var font := ThemeDB.fallback_font
	var line_height := 28.0
	var text := "OMT TEST | %s | frame %d | %0.2fs" % [pattern_name, frame_count, elapsed]
	draw_rect(Rect2(18, 18, min(size.x - 36.0, 760.0), 44), Color(0, 0, 0, 0.65))
	draw_string(font, Vector2(34, 48), text, HORIZONTAL_ALIGNMENT_LEFT, -1, line_height, Color.WHITE)
