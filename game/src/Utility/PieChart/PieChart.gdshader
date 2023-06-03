shader_type canvas_item;

// The center in UV coordinates, which will always
//be 0.5, the actual radius will be controlled by the control node
//const vec2 center = vec2(0.5,0.5);
uniform float radius = 0.4;

//shadow
uniform vec2 shadow_displacement = vec2(0.03,0.05);
uniform float shadow_rel_size = 0.43;
uniform float shadow_falloff = 0.5;
uniform float shadow_base = 2;

// Control of the slices
uniform float stopAngles[5];
uniform vec3 colours[5];

// Trim
uniform vec3 trim_colour;
uniform float trim_size = 0.05;

// The center is spotlighted by the gradient,
//control its size and falloff with these
uniform float gradient_falloff = 3.6;
uniform float gradient_base = 3.1;

// control whether this is a donut instead of a pie chart
uniform bool donut = false;
uniform bool donut_inner_trim = false;
uniform float donut_inner_radius = 0.15;

// get the polar coordinates of a pixel relative to the center
vec2 getPolar(vec2 UVin, vec2 center){
	vec2 relcoord = (UVin-center);
	float dist = length(relcoord);
	float theta = PI/2.0 + atan((relcoord.y)/(relcoord.x));
	if(UVin.x < 0.5){
		theta += PI;
	}
	return vec2(dist,theta);
}

// from thebookofshaders, returns a gradient falloff
float parabola( float base, float x, float k ){
    return pow( base*x*(1.0-x), k );
}

//lets try to make a new, proper parabola

bool isShadow(){
	return false;
}

uniform float xyz = 1.0;
//(radius-donut_inner_radius)/2.0
void fragment() {
	vec2 coords = getPolar(UV,vec2(0.5,0.5));
	float dist = coords.x;
	float theta = coords.y;
	
	vec2 shadow_polar = getPolar(UV,vec2(0.5+shadow_displacement.x,0.5+shadow_displacement.y));
	float shadow_gradient = 0.5*(0.7*parabola(shadow_base,
	(shadow_polar.x-xyz),-shadow_falloff)); //+ 0.3*parabola((shadow_polar.x),shadow_falloff));
	// inner hole of the donut => make it trasparent
	if(donut && dist <= donut_inner_radius){
		if(shadow_polar.x < shadow_rel_size * radius && shadow_polar.x > shadow_rel_size * donut_inner_radius){
			COLOR = vec4(0.1,0.1,0.1,0.7*shadow_gradient);
		}
		else{
			COLOR = vec4(0.0,0.0,0.0,0.0);
		}
		
	}
	// inner trim
	else if(donut && donut_inner_trim && dist <= donut_inner_radius + trim_size){
		COLOR = vec4(trim_colour,1.0);
	}
	// interior
	else if(dist <= radius-trim_size){ 
		for(int i=0;i<stopAngles.length();i++){
			if(theta <= stopAngles[i]){
				float gradient = parabola(gradient_base,dist,gradient_falloff);
				COLOR = vec4(colours[i]*(1.0-gradient),1.0);
				break;
			}
		}
	}
	// outer trim
	else if(dist <= radius){ 
		COLOR = vec4(trim_colour,1.0);
	}
	//outside the circle, check if we want a drop shadow
	else if(shadow_polar.x < shadow_rel_size * radius){
		if(donut && shadow_polar.x > shadow_rel_size * donut_inner_radius){
			COLOR = vec4(0.1,0.1,0.1,0.7*shadow_gradient);
		}
		else{
			COLOR = vec4(0.0,0.0,0.0,0.0);
		}
	}
	else{ //outside and don't want a dropshadow
		COLOR = vec4(0.0,0.0,0.0,0.0);
	}
}