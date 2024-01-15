shader_type spatial;

render_mode unshaded;

#include "res://src/Game/GameSession/ProvinceIndexSampler.gdshaderinc"

// The samplers below do not have the source_color hint because we do not want them
// to be converted from sRGB to linear colour space, we do that manually at the end.

// Province colour texture
uniform sampler2D province_colour_tex: repeat_enable, filter_nearest;
// Index of the mouse over the map mesh
uniform uint hover_index;
// Index of the currently selected province
uniform uint selected_index;
// Cosmetic terrain textures
uniform sampler2DArray terrain_tex: repeat_enable, filter_linear;
// The number of times the terrain textures should tile vertically
uniform float terrain_tile_factor;
// Map stripe mask texture
uniform sampler2D stripe_tex: repeat_enable, filter_linear;
// The number of times the stripe texture should tile vertically
uniform float stripe_tile_factor;
// Land map tint
uniform sampler2D colormap_land_tex: repeat_enable, filter_linear;
// Water map tint
uniform sampler2D colormap_water_tex: repeat_enable, filter_linear;

const vec3 highlight_colour = vec3(1.0);
// Rec.709 luma coefficients
const vec3 luma_weights = vec3(0.2126, 0.7152, 0.0722);

struct args_t {
	vec2 uv, half_pixel_size; // Components for calculating province sampling UV
	float stripe_mask; // Stripe mask value - between 0 (base) and 1 (stripe)
	vec2 terrain_uv; // UV coordinates scaled for terrain texture tiling
	vec3 land_tint_colour, water_tint_colour; // Colours for tinting terrain
};

// TODO - add parchment overlay, borders, coastlines, rivers, fog of war
// (when borders are added we can move province colour calculations out of get_terrain_colour
// and into get_map_colour, as we won't need to blend along the borders between provinces)

// This calculates the terrain and base-stripe province colours, blends them together, and then returns
// either the blended colour or just the terrain colour depending on whether the province colour's alpha
// value was set. It also highlights the result if the province is currently selected and/or hovered over.
vec3 get_terrain_colour(const args_t args, const vec2 corner) {
	// Find the province and terrain indices at the specified corner of a pixel centred on uv
	uvec3 province_data = read_uvec3(fma(corner, args.half_pixel_size, args.uv));
	uint province_index = uvec2_to_uint(province_data.rg);
	uint terrain_index = province_data.b;

	// Get the tinted terrain colour at the current position
	vec3 terrain_colour = texture(terrain_tex, vec3(args.terrain_uv, float(terrain_index))).rgb;
	vec3 tint_colour = mix(args.land_tint_colour, args.water_tint_colour, float(terrain_index == 0u));
	vec3 tinted_terrain_colour = mix(terrain_colour, tint_colour, 0.3);
	float terrain_luma = dot(tinted_terrain_colour, luma_weights);

	// Get base and stripe colours for province at the current position
	province_data.r *= 2u; // Double "x coordinate" as colours come in (base, stripe) pairs
	vec4 province_base_colour = texelFetch(province_colour_tex, ivec2(province_data.rg), 0);
	province_data.r += 1u; // Add 1 to "x coordinate" to move from base to strip colour
	vec4 province_stripe_colour = texelFetch(province_colour_tex, ivec2(province_data.rg), 0);
	// Blend the base and stripe colours according to the current position's stripe mask value
	vec4 province_colour = mix(province_base_colour, province_stripe_colour, args.stripe_mask);

	// Darken the province colour, use it to tint the monochrome terrain colour, and re-lighten the result
	province_colour.rgb = mix(vec3(terrain_luma), province_colour.rgb - 0.7, 0.3) * 1.5;

	// Show the terrain colour if the province colour has no alpha, otherwise show the province colour
	vec3 mixed_colour = mix(tinted_terrain_colour, province_colour.rgb, float(province_colour.a != 0.0));

	float highlight_mix_val = 0.4 * (float(province_index == hover_index) + float(province_index == selected_index)) * float(province_index != 0u);
	return mix(mixed_colour, highlight_colour, highlight_mix_val);
}

// Rescale UV coordinates to remove squashing caused by normalisation
vec2 denormalise(const vec2 uv, const vec2 dims) {
	return vec2(uv.x * dims.x / dims.y, uv.y);
}

// Calculate position-specific values and then calculate and blend map colours at the four corners
// of a pixel centred at uv in order to have smooth transitions between terrain types.
vec3 get_map_colour(vec2 uv) {
	args_t args;
	args.uv = uv;

	vec2 map_size = vec2(textureSize(province_shape_tex, 0).xy) * province_shape_subdivisions;
	vec2 uv_centred = fma(uv, map_size, vec2(0.5));
	vec2 pixel_offset = fract(uv_centred);
	args.half_pixel_size = 0.49 / map_size;

	// UV coords adjusted to remove squashing caused by normalisation relative to map dimensions
	vec2 unscaled_uv = denormalise(uv, map_size);

	vec2 stripe_uv = unscaled_uv * stripe_tile_factor;
	// Stripe mask value - between 0 (base) and 1 (stripe)
	args.stripe_mask = texture(stripe_tex, stripe_uv).b;

	args.terrain_uv = 0.5 - unscaled_uv * terrain_tile_factor;
	vec2 colormap_uv = vec2(uv.x, 1.0 - uv.y);
	args.land_tint_colour = texture(colormap_land_tex, colormap_uv).rgb;
	args.water_tint_colour = texture(colormap_water_tex, colormap_uv).rgb;

	return mix(
		mix(get_terrain_colour(args, vec2(-1, -1)), get_terrain_colour(args, vec2(+1, -1)), pixel_offset.x),
		mix(get_terrain_colour(args, vec2(-1, +1)), get_terrain_colour(args, vec2(+1, +1)), pixel_offset.x),
		pixel_offset.y
	);
}

// Convert from standard RGB to linear colour space (Godot requires the output to be linear)
vec3 srgb_to_linear(vec3 srgb) {
	return mix(
		srgb * (1.0 / 12.92),
		pow((srgb + vec3(0.055)) * (1.0 / (1.0 + 0.055)), vec3(2.4)),
		greaterThan(srgb, vec3(0.04045))
	);
}

void fragment() {
	ALBEDO = srgb_to_linear(get_map_colour(UV));
}
