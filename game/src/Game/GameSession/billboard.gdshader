shader_type spatial;
//depth_test_disabled along with the sorting offset of 10
//ensures this will be drawn in front of other objects in 
//3d space.
render_mode unshaded, depth_test_disabled;

//vic2 only ever loads a max of 12 BillboardType
//Of these, only 4 are actually used
uniform sampler2D billboards[12] : source_color;
uniform uint numframes[12];
uniform float sizes[12];

//COLOR/INSTANCE_CUSTOM is our custom data, used as follows:
// x=image index
// y=frame in image index
// z,w are unused for now

void vertex() {
	COLOR = INSTANCE_CUSTOM; //send instance_custom info to fragment
	float size = sizes[uint(COLOR.x + 0.5)];
	VERTEX = (vec4(VERTEX * size, 1.0) * VIEW_MATRIX).xyz;
}

void fragment() {
	uint image_index = uint(COLOR.x + 0.5);
	//Color.y == 0 >> invisible, otherwise use the specified frame
	//COLOR.y is frame index otherwise
	float visibility = min(COLOR.y,1.0);

	float uv_x_space = 1.0/float(numframes[image_index]);
	vec2 uv_out = vec2(uv_x_space * (COLOR.y + UV.x), UV.y);
	vec4 billboard_color = texture( billboards[image_index] ,uv_out);
	ALBEDO.rgb = billboard_color.rgb;
	ALPHA = billboard_color.a * visibility;
}
