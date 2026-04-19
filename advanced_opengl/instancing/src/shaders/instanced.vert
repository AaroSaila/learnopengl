#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 2) in vec2 a_tex_coords;
layout (location = 3) in mat4 instance_matrix;

uniform mat4 view;
uniform mat4 projection;

out vec2 tex_coords;

void main() {
   gl_Position = projection * view * instance_matrix * vec4(a_pos, 1.0f);
   tex_coords = a_tex_coords;
}
