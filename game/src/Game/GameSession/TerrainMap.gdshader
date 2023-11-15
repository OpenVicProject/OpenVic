shader_type spatial;

render_mode unshaded;

#include "res://src/Game/GameSession/ProvinceIndexSampler.gdshaderinc"

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
// Map stripe mask texture
uniform sampler2D stripe_tex: source_color, repeat_enable, filter_nearest;
// The number of times the stripe texture should tile vertically
uniform float stripe_tile_factor;

const vec3 highlight_colour = vec3(1.0);

vec3 get_terrain_colour(vec2 uv, vec2 corner, vec2 half_pixel_size, vec2 terrain_uv, vec2 stripe_uv) {
	uvec3 province_data = read_uvec3(fma(corner, half_pixel_size, uv));
	uint province_index = uvec2_to_uint(province_data.rg);
	uint terrain_index = province_data.b;

	province_data.r *= 2u; // Double "x coordinate" as colours come in (base, stripe) pairs
	vec4 province_base_colour = texelFetch(province_colour_tex, ivec2(province_data.rg), 0);
	province_data.r += 1u; // Add 1 to "x coordinate" to move from base to strip colour
	vec4 province_stripe_colour = texelFetch(province_colour_tex, ivec2(province_data.rg), 0);
	float stripe = texture(stripe_tex, stripe_uv).b; // Stripe mask value - 0 for base, 1 for stripe
	vec4 province_colour = mix(province_base_colour, province_stripe_colour, stripe);

	vec3 terrain_colour = texture(terrain_tex, vec3(terrain_uv, float(terrain_index))).rgb;
	vec3 mixed_colour = mix(terrain_colour, province_colour.rgb, province_colour.a);

	float mix_val = 0.1 * (float(province_index == hover_index) + float(province_index == selected_index));
	return mix(mixed_colour, highlight_colour, mix_val);
}

vec3 mix_terrain_colour(vec2 uv) {
	vec2 map_size = vec2(textureSize(province_shape_tex, 0).xy) * province_shape_subdivisions;
	vec2 pixel_offset = fract(fma(uv, map_size, vec2(0.5)));
	vec2 half_pixel_size = 0.49 / map_size;

	// UV coords adjusted to remove squashing caused by normalisation relative to map dimensions.
	vec2 unscaled_uv = vec2(uv.x * map_size.x / map_size.y, uv.y);

	vec2 terrain_uv = unscaled_uv * terrain_tile_factor;
	vec2 stripe_uv = unscaled_uv * stripe_tile_factor;

	return mix(
		mix(get_terrain_colour(uv, vec2(-1, -1), half_pixel_size, terrain_uv, stripe_uv),
			get_terrain_colour(uv, vec2(+1, -1), half_pixel_size, terrain_uv, stripe_uv), pixel_offset.x),
		mix(get_terrain_colour(uv, vec2(-1, +1), half_pixel_size, terrain_uv, stripe_uv),
			get_terrain_colour(uv, vec2(+1, +1), half_pixel_size, terrain_uv, stripe_uv), pixel_offset.x),
			pixel_offset.y);
}

void fragment() {
	vec3 terrain_colour = mix_terrain_colour(UV);
	ALBEDO = terrain_colour;
}
