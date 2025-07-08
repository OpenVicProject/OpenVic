shader_type spatial;

// depth_prepass_alpha is to ensure opaque scrolling textures
// (e.g. tank tracks) are rendered correctly
render_mode cull_disabled, depth_prepass_alpha;

uniform sampler2D scroll_texture_diffuse[32] : source_color, filter_linear_mipmap, repeat_enable;
uniform float scroll_factor[32];

instance uniform uint scroll_tex_index_diffuse;
instance uniform float scroll_speed;

void fragment() {
	vec2 uv_scrolled = UV;
	uv_scrolled.y += TIME * scroll_speed * scroll_factor[scroll_tex_index_diffuse];

	ALBEDO = texture(scroll_texture_diffuse[scroll_tex_index_diffuse], uv_scrolled).rgb;
	ALPHA = texture(scroll_texture_diffuse[scroll_tex_index_diffuse], UV).a;
}
