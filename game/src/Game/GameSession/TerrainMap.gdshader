shader_type spatial;

render_mode unshaded;

#include "res://src/GameSession/ProvinceIndexSampler.gdshaderinc"

// Province colour texture
uniform sampler2D province_colour_tex: source_color, repeat_enable, filter_nearest;
// Index of the mouse over the map mesh
uniform uint hover_index;
// Index of the currently selected province
uniform uint selected_index;
// Cosmetic terrain textures
uniform sampler2DArray terrain_tex: source_color, repeat_enable, filter_linear;
// The number of times the terrain textures should tile vertically
uniform float terrain_tile_factor;

const vec3 highlight_colour = vec3(1.0);

vec3 get_terrain_colour(vec2 uv, vec2 corner, vec2 half_pixel_size, vec2 terrain_uv) {
	uvec3 province_data = read_uvec3(fma(corner, half_pixel_size, uv));
	vec4 province_colour = texelFetch(province_colour_tex, ivec2(province_data.rg), 0);
	vec3 terrain_colour = texture(terrain_tex, vec3(terrain_uv, float(province_data.b))).rgb;
	vec3 mixed_colour = mix(terrain_colour, province_colour.rgb, province_colour.a);
	uint index = uvec2_to_uint(province_data.rg);
	float mix_val = 0.1 * (float(index == hover_index) + float(index == selected_index));
	return mix(mixed_colour, highlight_colour, mix_val);
}

vec3 mix_terrain_colour(vec2 uv) {
	vec2 map_size = vec2(textureSize(province_shape_tex, 0).xy) * province_shape_subdivisions;
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
