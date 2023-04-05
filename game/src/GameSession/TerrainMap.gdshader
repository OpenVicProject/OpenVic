shader_type spatial;

render_mode unshaded;

// Cosmetic terrain texture
uniform sampler2D terrain_tex: source_color, repeat_enable;
// Province shape texture
uniform sampler2D province_tex: source_color, repeat_enable;
// Position of the mouse over the map mesh in UV coords
uniform vec2 hover_pos;
// Position in UV coords of a pixel belonging to the currently selected province
uniform vec2 selected_pos;

// Transform map mesh UV coordinates to account for the extra
// half map on either side. This takes the x-coord from [0 -> 1]
// to [-0.5 -> 1.5], while leaving the y-coord unchanged.
vec2 fix_uv(vec2 uv) {
	return vec2(uv.x * 2.0 - 0.5, uv.y);
}

const vec3 NULL_COLOUR = vec3(0.0);

void fragment() {
	vec2 fixed_uv = fix_uv(UV);
	vec3 terrain_colour = texture(terrain_tex, fixed_uv).rgb;
	vec3 prov_colour = texture(province_tex, fixed_uv).rgb;
	vec3 hover_colour = texture(province_tex, fix_uv(hover_pos)).rgb;
	vec3 selected_colour = texture(province_tex, fix_uv(selected_pos)).rgb;
	// Boost prov_colour's contribution if it matches hover_colour or selected_colour
	float mix_val = float(prov_colour == hover_colour) * 0.3 + float(prov_colour == selected_colour) * 0.5;
	// Set to 0 if the province has NULL colour
	mix_val *= 1.0 - float(prov_colour ==  NULL_COLOUR);
	ALBEDO = mix(terrain_colour, prov_colour, mix_val);
}
