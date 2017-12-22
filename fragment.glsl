#version 300 es

// Default fragment shader

precision highp float;

out vec4 color;

in vec2 UV;

uniform float time;

vec3 light_color = vec3(1.0f,1.0f,1.0f);

void main(){

	vec2 p = UV - 0.5;
	
	float r = 0.1 + p.y;
	float g = 0.3;
	float b = 0.5;

	float f = 0.003 * cos(p.x * 30.0 + time * 6.28);
	f += 0.02 * cos(p.x * 4.0 + time * 6.28);
	f += 0.01 * cos(p.x * 8.0 + time * 6.28);
	
	if(p.y < f){
		r += 1.0;
	}
	
	color = vec4(r, g, b, 1.0);
}
