
shader_type spatial;

render_mode cull_disabled, depth_prepass_alpha;

//hold all the textures for the units that need this shader to mix in their
//nation colours (mostly generic infantry units)
//TODO: decrease back to 32
uniform sampler2D texture_diffuse[64] : source_color, filter_linear_mipmap, repeat_enable;
uniform sampler2D texture_nation_colors_mask[64] : source_color, filter_linear_mipmap, repeat_enable;
//uniform sampler2D texture_shadow[32] : source_color, filter_linear_mipmap, repeat_enable;

instance uniform vec3 colour_primary : source_color;
instance uniform vec3 colour_secondary : source_color;
instance uniform vec3 colour_tertiary : source_color;

//used to access the right textures since different units (with different textures)
//will use this same shader
instance uniform int tex_index_diffuse;
instance uniform int tex_index_specular;
//instance uniform int tex_index_shadow = -1; //0 = none

void fragment() {
	vec2 base_uv = UV;
	vec4 diffuse_tex = texture(texture_diffuse[tex_index_diffuse], base_uv);
	vec4 nation_colours_tex = texture(texture_nation_colors_mask[tex_index_specular], base_uv);
	
	//set colours to either be white (1,1,1) or the nation colour based on the mask
	vec3 primary_col = mix(vec3(1.0, 1.0, 1.0), colour_primary, nation_colours_tex.g);
	vec3 secondary_col = mix(vec3(1.0, 1.0, 1.0), colour_secondary, nation_colours_tex.b);
	vec3 tertiary_col = mix(vec3(1.0, 1.0, 1.0), colour_tertiary, nation_colours_tex.r);
	
	ALBEDO = diffuse_tex.rgb * primary_col * secondary_col * tertiary_col;
	ALPHA = diffuse_tex.a;
	/*if(tex_index_shadow != -1){
		vec4 shadow_tex = texture(texture_shadow[tex_index_shadow], base_uv);
		ALBEDO = shadow_tex.rgb;
	}*/
}
