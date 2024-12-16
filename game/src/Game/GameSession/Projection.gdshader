shader_type spatial;
render_mode unshaded;

uniform sampler2D projections[3] : source_color;
uniform float sizes[3];
uniform float spin[3];
uniform float expanding[3];
uniform float duration[3];
uniform bool additative[3]; //if true, black becomes a transparency colour
uniform float time[3];

void vertex() {
	COLOR = INSTANCE_CUSTOM;
	uint type = uint(COLOR.x + 0.5);
	float start_time = COLOR.y;

	float vertex_size = clamp(
		expanding[type] * (time[type]-start_time),
		0.0,
		sizes[type]
	);

	float rot = time[type]*spin[type];
	float cos_rot = cos(rot) * vertex_size;
	float sin_rot = sin(rot) * vertex_size;
	mat3 rotation_matrix = mat3(
		vec3(cos_rot, 0.0, -sin_rot),
		vec3(0.0, vertex_size, 0.0),
		vec3(sin_rot, 0.0, cos_rot)
	);
	VERTEX.xyz *= rotation_matrix;
}

void fragment() {
	// Called for every pixel the material is visible on.
	uint type = uint(COLOR.x + 0.5);
	vec4 sample = texture(projections[type],UV);
	ALBEDO.rgb = sample.rgb;

	float start_time = COLOR.y;
	//duration == 0 -> 1, duration == 1 -> 0
	float unlimited = float(duration[type] == 0.0);
	float is_finished = unlimited + ceil(clamp(duration[type] - time[type] + start_time,0.0,1.0));

	//if additative (bool), then black = transparent, otherwise use alpha
	float additative_transparency = float(additative[type]);
	ALPHA = COLOR.z * is_finished * mix(
		sample.a,
		sample.r+sample.g+sample.b,
		additative_transparency
	);
}