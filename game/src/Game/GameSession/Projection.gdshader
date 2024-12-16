shader_type spatial;
render_mode unshaded;

uniform sampler2D projections[3] : source_color;
uniform float sizes[3];
uniform float spin[3];
uniform float expanding[3];
uniform float duration[3];
uniform bool additative[3]; //if true, black becomes a transparency colour
uniform float time = 0.0;

void vertex() {
	COLOR = INSTANCE_CUSTOM;
	uint type = uint(COLOR.x + 0.5);
	float start_time = COLOR.y;

	float rot = time*spin[type];
	mat3 rotation_matrix = mat3(
		vec3(cos(-rot), 0.0, sin(-rot)),
		vec3(0.0, 1.0, 0.0),
		vec3(-sin(-rot), 0.0, cos(-rot))
	);
	VERTEX.xyz *= rotation_matrix * 
		clamp(
			expanding[type] * (time-start_time),
			0.0,
			sizes[type]
		);
}

void fragment() {
	// Called for every pixel the material is visible on.
	uint type = uint(COLOR.x + 0.5);
	vec4 sample = texture(projections[type],UV);
	ALBEDO.rgb = sample.rgb;

	float start_time = COLOR.y;
	//duration == 0 -> 1, duration == 1 -> 0
	float unlimited = float(duration[type] == 0.0);
	float is_finished = unlimited + ceil(clamp(duration[type] - time + start_time,0.0,1.0));

	//if additative, then black = transparent, otherwise use alpha
	float add_tr = float(additative[type]);
	ALPHA = is_finished*((1.0 - add_tr)*sample.a + add_tr*(sample.r+sample.g+sample.b));
}