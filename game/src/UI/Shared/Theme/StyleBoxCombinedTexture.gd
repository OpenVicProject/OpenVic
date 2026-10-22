@tool
class_name StyleBoxCombinedTexture
extends StyleBox

@export
var texture_settings: Array[TextureSetting] = []:
	get: return texture_settings.duplicate()
	set(v):
		# Disconnect old settings so removed resources cannot keep invalidating this stylebox.
		for setting: TextureSetting in texture_settings:
			if setting != null and setting.changed.is_connected(emit_changed):
				setting.changed.disconnect(emit_changed)

		texture_settings = v
		for setting: TextureSetting in texture_settings:
			if setting != null and not setting.changed.is_connected(emit_changed):
				setting.changed.connect(emit_changed)
		emit_changed()


func _draw(to_canvas_item: RID, rect: Rect2) -> void:
	for setting: TextureSetting in texture_settings:
		if setting == null or setting.texture == null:
			continue
		var inner_rect: Rect2 = rect
		inner_rect.position.x -= setting.expand_margin_left
		inner_rect.position.y -= setting.expand_margin_top
		inner_rect.size.x += setting.expand_margin_left + setting.expand_margin_right
		inner_rect.size.y += setting.expand_margin_top + setting.expand_margin_bottom
		RenderingServer.canvas_item_add_nine_patch(
			to_canvas_item,
			inner_rect,
			setting.region_rect,
			setting.texture.get_rid(),
			Vector2(setting.texture_margin_left, setting.texture_margin_top),
			Vector2(setting.texture_margin_right, setting.texture_margin_bottom),
			setting.axis_stretch_horizontal,
			setting.axis_stretch_vertical,
			setting.draw_center,
			setting.modulate_color
		)


func _get_style_margin(margin: Side) -> float:
	var combined_margin: float = 0.0
	for setting: TextureSetting in texture_settings:
		if setting == null:
			continue
		var content_margin: float
		var texture_margin: float
		match margin:
			SIDE_LEFT:
				content_margin = setting.content_margin_left
				texture_margin = setting.texture_margin_left
			SIDE_TOP:
				content_margin = setting.content_margin_top
				texture_margin = setting.texture_margin_top
			SIDE_RIGHT:
				content_margin = setting.content_margin_right
				texture_margin = setting.texture_margin_right
			SIDE_BOTTOM:
				content_margin = setting.content_margin_bottom
				texture_margin = setting.texture_margin_bottom
			_:
				continue
		# Like StyleBoxTexture, a negative content margin falls back to the texture margin.
		if content_margin < 0.0:
			content_margin = texture_margin
		combined_margin = maxf(combined_margin, content_margin)
	return combined_margin


func _get_draw_rect(rect: Rect2) -> Rect2:
	# Use the largest outward margin on each edge so no texture layer is clipped.
	var left_margin: float = 0.0
	var top_margin: float = 0.0
	var right_margin: float = 0.0
	var bottom_margin: float = 0.0
	for setting: TextureSetting in texture_settings:
		if setting == null:
			continue
		left_margin = maxf(left_margin, setting.expand_margin_left)
		top_margin = maxf(top_margin, setting.expand_margin_top)
		right_margin = maxf(right_margin, setting.expand_margin_right)
		bottom_margin = maxf(bottom_margin, setting.expand_margin_bottom)
	return rect.grow_individual(left_margin, top_margin, right_margin, bottom_margin)
