shader_type spatial;

render_mode unshaded;

uniform sampler2D tex: source_color, repeat_enable;

void fragment() {
	vec2 new_uv = vec2(UV.x * 2.0 - 0.5, UV.y);
	ALBEDO = texture(tex, new_uv).rgb;
}
