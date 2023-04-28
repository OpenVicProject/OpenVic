
// Province shape texture
uniform sampler2DArray province_shape_tex : repeat_enable, filter_nearest;
// Province shape subdivisions
uniform vec2 province_shape_subdivisions;

uvec3 vec3_to_uvec3(vec3 v) {
	return uvec3(v * 255.0);
}
uvec3 read_uvec3(vec2 uv) {
	uv *= province_shape_subdivisions;
	vec2 subdivision_coords = mod(floor(uv),  province_shape_subdivisions);
	float idx = subdivision_coords.x + subdivision_coords.y * province_shape_subdivisions.x;
	return vec3_to_uvec3(texture(province_shape_tex, vec3(uv, idx)).rgb);
}
uint uvec2_to_uint(uvec2 v) {
	return (v.y << 8u) | v.x;
}
