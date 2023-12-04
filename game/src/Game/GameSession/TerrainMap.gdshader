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
uniform sampler2D stripe_tex: source_color, repeat_enable, filter_linear;
// The number of times the stripe texture should tile vertically
uniform float stripe_tile_factor;
// Land map tint
uniform sampler2D colormap_land_tex: source_color, repeat_enable, filter_linear;
// Water map tint
uniform sampler2D colormap_water_tex: source_color, repeat_enable, filter_linear;

const vec3 highlight_colour = vec3(1.0);

vec3 get_terrain_colour(
	vec2 uv, vec2 corner, vec2 half_pixel_size, // Components for calculating province sampling UV
	float stripe_mask, // Stripe mask value - between 0 (base) and 1 (stripe)
	vec2 terrain_uv, // UV coordinates scaled for terrain texture tiling
	vec3 land_tint_colour, vec3 water_tint_colour // Colours for tinting terrain
) {

	uvec3 province_data = read_uvec3(fma(corner, half_pixel_size, uv));
	uint province_index = uvec2_to_uint(province_data.rg);
	uint terrain_index = province_data.b;

	province_data.r *= 2u; // Double "x coordinate" as colours come in (base, stripe) pairs
	vec4 province_base_colour = texelFetch(province_colour_tex, ivec2(province_data.rg), 0);
	province_data.r += 1u; // Add 1 to "x coordinate" to move from base to strip colour
	vec4 province_stripe_colour = texelFetch(province_colour_tex, ivec2(province_data.rg), 0);
	vec4 province_colour = mix(province_base_colour, province_stripe_colour, stripe_mask);

	vec3 terrain_colour = texture(terrain_tex, vec3(terrain_uv, float(terrain_index))).rgb;
	vec3 tint_colour = mix(land_tint_colour, water_tint_colour, float(terrain_index == 0u));
	vec3 tinted_terrain_colour = mix(terrain_colour, tint_colour, 0.3);
	vec3 mixed_colour = mix(tinted_terrain_colour, province_colour.rgb, province_colour.a);

	float highlight_mix_val = 0.4 * (float(province_index == hover_index) + float(province_index == selected_index));
	return mix(mixed_colour, highlight_colour, highlight_mix_val);
}

// Rescale UV coordinates to remove squashing caused by normalisation
vec2 denormalise(vec2 uv, vec2 dims) {
	return vec2(uv.x * dims.x / dims.y, uv.y);
}

vec3 mix_terrain_colour(vec2 uv) {
	vec2 map_size = vec2(textureSize(province_shape_tex, 0).xy) * province_shape_subdivisions;
	vec2 uv_centred = fma(uv, map_size, vec2(0.5));
	vec2 pixel_offset = fract(uv_centred);
	vec2 half_pixel_size = 0.49 / map_size;

	// UV coords adjusted to remove squashing caused by normalisation relative to map dimensions
	vec2 unscaled_uv = denormalise(uv, map_size);

	vec2 stripe_uv = unscaled_uv * stripe_tile_factor;
	// Stripe mask value - between 0 (base) and 1 (stripe)
	float stripe_mask = texture(stripe_tex, stripe_uv).b;

	vec2 terrain_uv = unscaled_uv * terrain_tile_factor;
	vec2 colormap_uv = vec2(uv.x, 1.0 - uv.y);
	vec3 colormap_land_colour = texture(colormap_land_tex, colormap_uv).rgb;
	vec3 colormap_water_colour = texture(colormap_water_tex, colormap_uv).rgb;

	return mix(
		mix(
			get_terrain_colour(
				uv, vec2(-1, -1), half_pixel_size, stripe_mask,
				terrain_uv, colormap_land_colour, colormap_water_colour
			),
			get_terrain_colour(
				uv, vec2(+1, -1), half_pixel_size, stripe_mask,
				terrain_uv, colormap_land_colour, colormap_water_colour
			), pixel_offset.x
		),
		mix(
			get_terrain_colour(
				uv, vec2(-1, +1), half_pixel_size, stripe_mask,
				terrain_uv, colormap_land_colour, colormap_water_colour
			),
			get_terrain_colour(
				uv, vec2(+1, +1), half_pixel_size, stripe_mask,
				terrain_uv, colormap_land_colour, colormap_water_colour
			), pixel_offset.x
		),
		pixel_offset.y
	);
}

void fragment() {
	vec3 terrain_colour = mix_terrain_colour(UV);
	ALBEDO = terrain_colour;
}
