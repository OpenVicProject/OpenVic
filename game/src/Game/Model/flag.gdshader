shader_type spatial;

render_mode cull_disabled;

// Both vanilla flags use the same normal texture
uniform uvec2 flag_dims;
uniform sampler2D texture_flag_sheet_diffuse : source_color;
uniform sampler2D texture_normal : hint_normal;

instance uniform uint flag_index;

uniform vec2 scroll_speed = vec2(-0.25,0);

// Scroll the Normal map, but leave the albedo alone
void fragment() {
	uvec2 flag_sheet_dims = uvec2(textureSize(texture_flag_sheet_diffuse, 0));
	uint scaled_index = flag_index * flag_dims.x;

	uvec2 flag_pos = uvec2(scaled_index % flag_sheet_dims.x, scaled_index / flag_sheet_dims.x * flag_dims.y);

	vec2 flag_uv = (vec2(flag_pos) + UV * vec2(flag_dims)) / vec2(flag_sheet_dims);

	ALBEDO = texture(texture_flag_sheet_diffuse, flag_uv).rgb;
	//ALBEDO = vec3(1, 0, 0);
	NORMAL_MAP = texture(texture_normal, UV + TIME*scroll_speed).rgb;
}
