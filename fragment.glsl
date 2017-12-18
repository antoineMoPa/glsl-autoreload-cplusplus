#version 300 es

// Default fragment shader

precision highp float;

out vec4 color;

in vec2 UV;

vec3 light_color = vec3(1.0f,1.0f,1.0f);

void main(){
	float r = UV.x;
	float g = 0.0;
	float b = 1.0;
	
	color = vec4(r, g, b, 1.0);
}
