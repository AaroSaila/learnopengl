#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;

uniform float horizontal_offset;

out vec3 our_color;

void main() {
   gl_Position = vec4(a_pos.x + horizontal_offset, a_pos.yz, 1.0);
   our_color = a_color;
}
