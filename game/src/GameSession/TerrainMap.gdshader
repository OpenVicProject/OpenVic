shader_type spatial;

render_mode unshaded;

// Cosmetic farmlands terrain texture
uniform sampler2D farmlands_tex: source_color, repeat_enable, filter_linear;
// Province index texture
uniform sampler2DArray province_index_tex : source_color, repeat_enable, filter_nearest;
// Province index subdivisions
uniform vec2 province_index_subdivisions;
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
uvec2 read_uvec2(vec2 uv) {
	uv *= province_index_subdivisions;
	vec2 subdivision_coords = mod(floor(uv),  province_index_subdivisions);
	float idx = subdivision_coords.x + subdivision_coords.y * province_index_subdivisions.x;
	return vec2_to_uvec2(texture(province_index_tex, vec3(uv, idx)).rg);
}
uint uvec2_to_uint(uvec2 v) {
	return (v.y << 8u) | v.x;
}

const vec3 water_colour = vec3(0, 0, 1);

vec3 get_terrain_colour(vec2 uv, vec2 corner, vec2 half_pixel_size, vec2 terrain_uv) {
	uvec2 index_split = read_uvec2(fma(corner, half_pixel_size, uv));
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
	vec2 map_size = vec2(textureSize(province_index_tex, 0).xy);
	vec2 pixel_offset = fract(fma(uv, map_size, vec2(0.5)));
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
