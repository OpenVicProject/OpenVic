shader_type canvas_item;

#include "ProvinceIndexSampler.gdshaderinc"

void fragment() {
	COLOR.rgb = mix(COLOR.rgb, vec3(0.5), float(read_uvec3(UV).b != 0u));
}
