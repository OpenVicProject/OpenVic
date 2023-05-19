shader_type canvas_item;

/*void vertex() {
  VERTEX += vec2(10.0, 0.0);
}*/
/*struct Slice {
	float stop_angle;
	vec3 diffuse_colour;
};*/

uniform float stopAngles[5];
uniform vec3 colours[5];
uniform vec3 trim_colour;

const vec2 center = vec2(0.5,0.5);
const float radius = 0.5;
uniform float trim_size = 0.05;

uniform bool donut = false;
uniform bool donut_inner_trim = false;
uniform float donut_inner_radius = 0.15;

vec2 getPolar(vec2 UVin){
	vec2 relcoord = (UVin-center); /// dist;
	float dist = length(relcoord);
	float theta = PI/2.0 + atan((relcoord.y)/(relcoord.x));
	if(UVin.x < 0.5){
		theta += PI;
	}
	return vec2(dist,theta);
}

void fragment() {
	vec2 coords = getPolar(UV);
	float dist = coords.x;
	float theta = coords.y;
	//inner part of the donut => make it trasparent
	if(donut && dist <= donut_inner_radius){
		COLOR = vec4(0.0,0.0,0.0,0.0);
	}
	else if(donut && donut_inner_trim && dist <= donut_inner_radius + trim_size){
		COLOR = vec4(trim_colour,1.0);
	}
	else if(dist <= radius-trim_size){ //interior
		for(int i=0;i<stopAngles.length();i++){
			if(theta <= stopAngles[i]){
				COLOR = vec4(colours[i],1.0);
				break;
			}
		}
	}
	else if(dist <= radius){ //trim
		COLOR = vec4(trim_colour,1.0);
	}
	else { //outside the circle, make it transparent
		COLOR = vec4(0.0,0.0,0.0,0.0);
	}
	
}

/*

float plot(vec2 st, float pct){
	//create a kind of "trouph" of brightness that plots the line (so a hazy line in effect)
  return  smoothstep( pct-0.02, pct, st.y) -
          smoothstep( pct, pct+0.02, st.y);
}
	//xy coord
	vec2 st = UV.xy;
	//our gradient (black 0 to white 1)
	float y = smoothstep(0.1,0.9,st.x);
	//base colour (ie. b to w) at the coord
	vec3 color = vec3(y);
	
	//the green line plot
	float pct = plot(st,y);
	//color is the background color
	//(but the the line area set to black)
	// + the line in green
	color = (1.0-pct)*color + pct*vec3(0.0,1.0,0.0);
 	COLOR = vec4(color,1.0);
*/