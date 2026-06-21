#version 460 core

uniform mat4 u_view;
uniform mat4 u_model;
uniform mat4 u_projection;

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;

out vec3 world_pos;
out vec3 normal;
out vec2 tex_coords;

void main() {
    world_pos = vec3(u_model * vec4(a_pos, 1.0));
    gl_Position = u_projection * u_view * vec4(world_pos, 1.0);
    normal = transpose(inverse(mat3(u_model))) * a_normal;
    tex_coords = a_tex_coords;
}
