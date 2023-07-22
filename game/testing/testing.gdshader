shader_type spatial;
uniform sampler2D heightmap;
uniform float height_ration = 1.0;

void fragment() {
	// Place fragment code here.
}

void vertex() {
	VERTEX.y = texture(heightmap, UV).r * height_ration;
}
