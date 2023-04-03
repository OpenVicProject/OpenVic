shader_type spatial;

render_mode unshaded;

// Cosmetic terrain texture
uniform sampler2D terrain_tex: source_color, repeat_enable;
// Province shape texture
uniform sampler2D province_tex: source_color, repeat_enable;

// Mouse position in UV coords over the map mesh
uniform vec2 mouse_pos;

// Transform map mesh UV coordinates to account for the extra
// half map on either side. This takes the x-coord from [0 -> 1]
// to [-0.5 -> 1.5], while leaving the y-coord unchanged.
vec2 fix_uv(vec2 uv) {
	return vec2(uv.x * 2.0 - 0.5, uv.y);
}

void fragment() {
	vec2 fixed_uv = fix_uv(UV);
	vec3 prov_colour = texture(province_tex, fixed_uv).rgb;
	vec3 mouse_colour = texture(province_tex, fix_uv(mouse_pos)).rgb;
	// Boost prov_colour's contribution if the mouse is over that colour and it isn't (0,0,0)
	float mix_val = prov_colour == mouse_colour && mouse_colour != vec3(0.0) ? 0.8 : 0.4;
	ALBEDO = mix(texture(terrain_tex, fixed_uv).rgb, prov_colour, mix_val);
}
