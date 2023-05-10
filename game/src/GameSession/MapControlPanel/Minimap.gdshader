shader_type canvas_item;

#include "res://src/GameSession/ProvinceIndexSampler.gdshaderinc"

// Index of the currently selected province
uniform uint selected_index;

const vec3 land_colour = vec3(0.5);
const vec3 selected_colour = vec3(1.0, 1.0, 0.0);

void fragment() {
	uvec3 data = read_uvec3(UV);
	uint index = uvec2_to_uint(data.rg);
	float is_land = float(data.b != 0u);
	float is_selected = float(index == selected_index);
	COLOR.rgb = mix(COLOR.rgb, land_colour, is_land);
	COLOR.rgb = mix(COLOR.rgb, selected_colour, is_selected);
}
