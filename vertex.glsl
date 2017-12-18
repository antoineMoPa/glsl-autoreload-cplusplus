#version 300 es

// Default vertex shader

layout(location = 0) in vec3 v_pos;

out vec2 UV;

void main(){
	UV = (v_pos.xy + 1.0) / 2.0;
    gl_Position = vec4(v_pos, 1.0);
}
