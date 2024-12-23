@tool
extends StyleBox
class_name StyleBoxCombinedTexture

@export var texture_settings: Array[TextureSetting] = []:
	get:
		return texture_settings.duplicate()
	set(v):
		texture_settings = v
		for setting: TextureSetting in texture_settings:
			setting.changed.connect(emit_changed)
		emit_changed()


func _get_draw_rect(rect: Rect2) -> Rect2:
	var combined_rect: Rect2 = Rect2()
	for setting: TextureSetting in texture_settings:
		if combined_rect.position.x > setting.expand_margin_left:
			combined_rect.position.x = setting.expand_margin_left
		if combined_rect.position.y > setting.expand_margin_top:
			combined_rect.position.y = setting.expand_margin_top
		if combined_rect.end.x < setting.expand_margin_right:
			combined_rect.end.x = setting.expand_margin_right
		if combined_rect.end.y < setting.expand_margin_bottom:
			combined_rect.end.y = setting.expand_margin_bottom
	return rect.grow_individual(
		combined_rect.position.x, combined_rect.position.y, combined_rect.end.x, combined_rect.end.y
	)


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
