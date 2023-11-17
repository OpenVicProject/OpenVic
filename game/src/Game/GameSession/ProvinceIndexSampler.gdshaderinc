
// Province shape texture
uniform sampler2DArray province_shape_tex : repeat_enable, filter_nearest;
// Province shape subdivisions
uniform vec2 province_shape_subdivisions;

// Convert a vector of 3 normalised floats to a vector of 3 unsigned bytes
uvec3 vec3_to_uvec3(vec3 v) {
	return uvec3(v * 255.0);
}

// Create a uint triplet describing the province and terrain data at a map-space UV coordinate:
// (u, v) -> (province index bottom byte, province index top byte, terrain index byte)
uvec3 read_uvec3(vec2 uv) {
	uv *= province_shape_subdivisions;
	vec2 subdivision_coords = mod(floor(uv),  province_shape_subdivisions);
	float idx = subdivision_coords.x + subdivision_coords.y * province_shape_subdivisions.x;
	return vec3_to_uvec3(texture(province_shape_tex, vec3(uv, idx)).rgb);
}

// Combine a (lower byte, upper byte) uint pair into a single 2-byte uint
uint uvec2_to_uint(uvec2 v) {
	return (v.y << 8u) | v.x;
}
