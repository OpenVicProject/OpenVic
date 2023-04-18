shader_type spatial;

render_mode unshaded;

// Cosmetic farmlands terrain texture
uniform sampler2D farmlands_tex: source_color, repeat_enable, filter_linear;
// Province index texture
uniform sampler2D province_index_tex : source_color, repeat_enable, filter_nearest;
// Province colour texture
uniform sampler2D province_colour_tex: source_color, repeat_enable, filter_nearest;
// Index of the mouse over the map mesh
uniform uint hover_index;
// Index of the currently selected province
uniform uint selected_index;
// The number of times the terrain textures should tile vertically
uniform float terrain_tile_factor;

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

const vec3 water_colour = vec3(0, 0, 1);

vec3 get_terrain_colour(vec2 uv, vec2 corner, vec2 half_pixel_size, vec2 terrain_uv) {
	uvec2 index_split = read_uvec2(province_index_tex, fma(corner, half_pixel_size, uv));
	uint index = uvec2_to_uint(index_split);
	vec4 province_data = texelFetch(province_colour_tex, ivec2(index_split), 0);
	vec3 province_colour = province_data.rgb;
	float is_land = province_data.a;
	vec3 farmlands_colour = texture(farmlands_tex, terrain_uv).rgb;
	vec3 terrain_colour = mix(water_colour, farmlands_colour, is_land);
	float mix_val = 0.4 + float(index == hover_index) * 0.2 + float(index == selected_index) * 0.2;
	vec3 mixed_colour = mix(terrain_colour, province_colour, mix_val);
	return mixed_colour;
}

vec3 mix_terrain_colour(vec2 uv) {
	vec2 map_size = vec2(textureSize(province_index_tex, 0));
	vec2 pixel_offset = mod(fma(uv, map_size, vec2(0.5)), 1.0);
	vec2 half_pixel_size = 0.49 / map_size;

	vec2 terrain_uv = uv;
	terrain_uv.x *= map_size.x / map_size.y;
	terrain_uv *= terrain_tile_factor;

	return mix(
		mix(get_terrain_colour(uv, vec2(-1, -1), half_pixel_size, terrain_uv),
			get_terrain_colour(uv, vec2(+1, -1), half_pixel_size, terrain_uv), pixel_offset.x),
		mix(get_terrain_colour(uv, vec2(-1, +1), half_pixel_size, terrain_uv),
			get_terrain_colour(uv, vec2(+1, +1), half_pixel_size, terrain_uv), pixel_offset.x),
			pixel_offset.y);
}

void fragment() {
	vec3 terrain_colour = mix_terrain_colour(UV);
	ALBEDO = terrain_colour;
}
