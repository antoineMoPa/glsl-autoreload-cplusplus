#version 300 es

// Default fragment shader

precision highp float;

out vec4 color;

in vec2 UV;

uniform float time;

vec3 light_color = vec3(1.0f,1.0f,1.0f);

void main(){
	float r = 1.0;
	float g = 0.3 * cos(time * 10.0);
	float b = 0.0;
	
	color = vec4(r, g, b, 1.0);
}
