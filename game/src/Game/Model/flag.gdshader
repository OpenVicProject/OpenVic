shader_type spatial;

render_mode cull_disabled;

// Both vanilla flags use the same normal texture
uniform uvec2 flag_dims;
uniform sampler2D texture_flag_sheet_diffuse : source_color;
uniform sampler2D texture_normal : hint_normal;

instance uniform uint flag_index;

const float normal_scroll_speed = 0.3;

// Scroll the Normal map, but leave the albedo alone
void fragment() {
	uvec2 flag_sheet_dims = uvec2(textureSize(texture_flag_sheet_diffuse, 0));
	uint scaled_index = flag_index * flag_dims.x;

	uvec2 flag_pos = uvec2(scaled_index % flag_sheet_dims.x, scaled_index / flag_sheet_dims.x * flag_dims.y);

	vec2 flag_uv = (vec2(flag_pos) + UV * vec2(flag_dims)) / vec2(flag_sheet_dims);

	ALBEDO = texture(texture_flag_sheet_diffuse, flag_uv).rgb;

	vec2 normal_uv = UV;
	normal_uv.x -= TIME * normal_scroll_speed;

	NORMAL_MAP = texture(texture_normal, normal_uv).rgb;
}
