shader_type spatial;

render_mode unshaded;

// Cosmetic terrain texture
uniform sampler2D terrain_tex: source_color, repeat_enable, filter_linear;
// Province index texture
uniform sampler2D province_index_tex : repeat_enable, filter_nearest;
// Province colour texture
uniform sampler2D province_colour_tex: source_color, repeat_enable, filter_nearest;
// Index of the mouse over the map mesh
uniform uint hover_index;
// Index of the currently selected province
uniform uint selected_index;

uniform sampler2D basemap;
uniform sampler2D heightmap;
uniform float height_ratio: hint_range(0.0, 1.0, 0.002) = .005;
uniform float base_ratio: hint_range(0.0, 1.0, 0.002) = .001;



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

	// Boost prov_colour's contribution if it matches hover_colour or selected_colour
	float mix_val = 0.3 + float(prov_index == hover_index) * 0.3 + float(prov_index == selected_index) * 0.3;
	// Don't mix if the province index is 0
	mix_val *= float(prov_index != 0u);

	vec3 terrain_colour = texture(terrain_tex, UV).rgb;
	vec3 province_colour = texelFetch(province_colour_tex, ivec2(prov_idx_split), 0).rgb;
	ANISOTROPY = 1.0;
	ALBEDO = mix(terrain_colour, province_colour, mix_val);
}

void vertex() {
	VERTEX.y += texture(basemap, UV).r * base_ratio;
	VERTEX.y += texture(heightmap, UV).r * height_ratio;
}
