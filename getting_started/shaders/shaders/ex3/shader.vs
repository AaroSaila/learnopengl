#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;

out vec3 vertex_pos;

void main() {
   gl_Position = vec4(a_pos, 1.0);
   vertex_pos = a_pos;
}
