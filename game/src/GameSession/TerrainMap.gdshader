shader_type spatial;

render_mode unshaded;

// Cosmetic terrain texture
uniform sampler2D terrain_tex: source_color, repeat_enable, filter_linear;
// Province index texture
uniform sampler2D province_index_tex : repeat_enable, filter_nearest;
// Province colour texture
uniform sampler2D province_colour_tex: source_color, repeat_enable, filter_nearest;
// Position of the mouse over the map mesh in UV coords
uniform vec2 hover_pos;
// Position in UV coords of a pixel belonging to the currently selected province
uniform vec2 selected_pos;

uvec2 vec2_to_uvec2(vec2 v) {
	return uvec2(v * 255.0);
}

uint uvec2_to_uint(uvec2 v) {
	return (v.y << 8u) | v.x;
}

uvec2 read_uvec2(sampler2D tex, vec2 uv) {
	return vec2_to_uvec2(texture(tex, uv).rg);
}

uint read_uint16(sampler2D tex, vec2 uv) {
	return uvec2_to_uint(read_uvec2(tex, uv));
}

void fragment() {
	uvec2 prov_idx_split = read_uvec2(province_index_tex, UV);

	uint prov_index = uvec2_to_uint(prov_idx_split);
	uint hover_index = read_uint16(province_index_tex, hover_pos);
	uint selected_index = read_uint16(province_index_tex, selected_pos);

	// Boost prov_colour's contribution if it matches hover_colour or selected_colour
	float mix_val = 0.3 + float(prov_index == hover_index) * 0.3 + float(prov_index == selected_index) * 0.3;
	// Don't mix if the province index is 0
	mix_val *= float(prov_index != 0u);

	vec3 terrain_colour = texture(terrain_tex, UV).rgb;
	vec3 province_colour = texelFetch(province_colour_tex, ivec2(prov_idx_split), 0).rgb;

	ALBEDO = mix(terrain_colour, province_colour, mix_val);
}
