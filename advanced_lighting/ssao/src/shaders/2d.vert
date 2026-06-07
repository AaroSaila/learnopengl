#version 460 core

uniform mat4 model;

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coords;

out vec3 vo_frag_pos;
out vec2 vo_tex_coords;

void main() {
    gl_Position = vec4(vec3(model * vec4(a_pos, 1.0)), 1.0);
    vo_frag_pos = gl_Position.xyz;
    vo_tex_coords = a_tex_coords;
}
