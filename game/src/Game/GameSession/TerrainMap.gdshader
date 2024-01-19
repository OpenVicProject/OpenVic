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
// How much to mix between the near detailed map (0) and the far parchment map (1)
uniform float parchment_mix;
// Cosmetic terrain textures
uniform sampler2DArray terrain_tex: repeat_enable, filter_linear;
// Factor converting pixels to terrain tile size
uniform float terrain_tile_factor;
// Map stripe mask texture
uniform sampler2D stripe_tex: repeat_enable, filter_linear;
// Factor converting pixels to stripe tile size
uniform float stripe_tile_factor;
// Map parchment overlay texture
uniform sampler2D overlay_tex: repeat_enable, filter_linear;
// Factor converting pixels to overlay tile size
uniform float overlay_tile_factor;
// Land map tint
uniform sampler2D colormap_land_tex: repeat_enable, filter_linear;
// Water map tint
uniform sampler2D colormap_water_tex: repeat_enable, filter_linear;
// Overlay map tint
uniform sampler2D colormap_overlay_tex: repeat_enable, filter_linear;

struct terrain_args_t {
	vec2 uv, half_pixel_size; // Components for calculating terrain sampling UV
	vec2 terrain_uv; // UV coordinates scaled for terrain texture tiling
	vec3 land_tint_colour, water_tint_colour; // Colours for tinting the terrain
};

// Calculate terrain colour at the specified corner of the current pixel
vec3 get_terrain_colour(const terrain_args_t terrain_args, const vec2 corner) {
	// Find the terrain index at the specified corner of the current pixel
	uint terrain_index = read_uvec3(fma(corner, terrain_args.half_pixel_size, terrain_args.uv)).z;

	// Get the tinted land colour at the current position
	vec3 land_colour = texture(terrain_tex, vec3(terrain_args.terrain_uv, float(terrain_index))).rgb;
	land_colour = mix(land_colour, terrain_args.land_tint_colour, 0.3);

	// TODO - proper water texture
	vec3 water_colour = terrain_args.water_tint_colour;

	// Select land or water colour based on the terrain index (0 is water, otherwise land)
	vec3 terrain_colour = mix(land_colour, water_colour, float(terrain_index == 0u));

	return terrain_colour;
}

// Blend together terrain colours from the four corners of the current pixel
vec3 mix_terrain_colour(const terrain_args_t terrain_args, const vec2 pixel_offset) {
	return mix(
		mix(get_terrain_colour(terrain_args, vec2(-1, -1)), get_terrain_colour(terrain_args, vec2(+1, -1)), pixel_offset.x),
		mix(get_terrain_colour(terrain_args, vec2(-1, +1)), get_terrain_colour(terrain_args, vec2(+1, +1)), pixel_offset.x),
		pixel_offset.y
	);
}

// Mix overlay and base colours, used for the parchment map
vec3 mix_overlay(const vec3 overlay_colour, const vec3 base_colour) {
	return mix(
		2.0 * overlay_colour * base_colour,
		1.0 - 2.0 * (1.0 - overlay_colour) * (1.0 - base_colour),
		greaterThanEqual(overlay_colour.rrb, vec3(0.5))
	);
}

// Calculate map colour at specified UV coordinates, combining terrain, province base and stripe colour,
// parchment overlay, and highlighting the result if it is a hovered over and/or selected province.
vec3 get_map_colour(vec2 uv) {
	vec2 map_size = vec2(textureSize(province_shape_tex, 0).xy) * province_shape_subdivisions;
	vec2 uv_map_pixels = fma(uv, map_size, vec2(0.5));
	// Offset of uv_map_pixels from the top left corner of the current pixel
	vec2 pixel_offset = fract(uv_map_pixels);

	terrain_args_t terrain_args;
	terrain_args.uv = uv;
	terrain_args.half_pixel_size = 0.49 / map_size;
	// Terrain texture tiling UV
	terrain_args.terrain_uv = 0.5 - uv_map_pixels * terrain_tile_factor;

	vec2 colormap_uv = vec2(uv.x, 1.0 - uv.y);
	// Terrain tinting colours
	terrain_args.land_tint_colour = texture(colormap_land_tex, colormap_uv).rgb;
	terrain_args.water_tint_colour = texture(colormap_water_tex, colormap_uv).rgb;
	// Parchment tint colour
	vec3 overlay_tint_colour = texture(colormap_overlay_tex, colormap_uv).rgb;

	// Blended terrain colour (average of four corners of current pixel)
	vec3 terrain_colour = mix_terrain_colour(terrain_args, pixel_offset);

	vec2 stripe_uv = uv_map_pixels * stripe_tile_factor;
	// Stripe mask value - between 0 (base) and 1 (stripe)
	float stripe_mask = texture(stripe_tex, stripe_uv).b;

	vec2 overlay_uv = vec2(uv_map_pixels.x, map_size.y - uv_map_pixels.y) * overlay_tile_factor;
	// Parchment overlay colour
	vec3 overlay_colour = texture(overlay_tex, overlay_uv).rgb;

	// Current province index as a pair of byte coordinates and as a combined 16 bit value
	uvec2 province_data = read_uvec3(uv).xy;
	uint province_index = uvec2_to_uint(province_data);

	// Get base and stripe colours for province at the current position
	province_data.x *= 2u; // Double "x coordinate" as colours come in (base, stripe) pairs
	vec4 province_base_colour = texelFetch(province_colour_tex, ivec2(province_data), 0);
	province_data.x += 1u; // Add 1 to "x coordinate" to move from base to strip colour
	vec4 province_stripe_colour = texelFetch(province_colour_tex, ivec2(province_data), 0);
	// Blend the base and stripe colours according to the current position's stripe mask value
	vec4 province_colour = mix(province_base_colour, province_stripe_colour, stripe_mask);
	// Darken the province colour
	province_colour.rgb -= 0.7;

	// Rec.709 luma coefficients
	const vec3 luma_weights = vec3(0.2126, 0.7152, 0.0722);
	float terrain_luma = dot(terrain_colour, luma_weights);

	// Near colour is either the terrain's luma component tinted with the province colour and brightened,
	// or the normal terrain colour
	vec3 near_province_colour = mix(vec3(terrain_luma), province_colour.rgb, 0.3) * 1.5;
	vec3 near_colour = mix(near_province_colour, terrain_colour, float(province_colour.a == 0.0));

	// Far colour is either the province colour mixed with the overlay texture, tinted with the overlay colormap and brightened,
	// or the normal terrain colour mixed with the overlay texture (primarily for water)
	vec3 far_province_colour = mix_overlay(overlay_colour, province_colour.rgb);
	far_province_colour = mix(overlay_tint_colour, far_province_colour, 0.3) * 1.5;
	vec3 far_terrain_colour = mix_overlay(overlay_colour, terrain_colour);
	vec3 far_colour = mix(far_province_colour, far_terrain_colour, float(province_colour.a == 0.0));

	// Blend the near (detailed terrain) and far (parchment) colours according to the parchment mix factor (0 for near, 1 for far)
	vec3 final_colour = mix(near_colour, far_colour, parchment_mix);

	// Significantly brighted the colour if it is hovered over and/or selected, but not if it has province index 0 (all invalid pixels)
	const vec3 highlight_colour = vec3(1.0);
	float highlight_mix_val = 0.4 * (float(province_index == hover_index) + float(province_index == selected_index)) * float(province_index != 0u);
	vec3 highlighted_colour = mix(final_colour, highlight_colour, highlight_mix_val);

	return highlighted_colour;
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

	// TODO - add borders, coastlines, rivers, fog of war

	ALBEDO = srgb_to_linear(get_map_colour(UV));
}
