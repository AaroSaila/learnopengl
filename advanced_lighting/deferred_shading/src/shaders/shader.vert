#version 460 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

out vec3 vo_frag_pos;
out vec3 vo_normal;
out vec2 vo_tex_coords;

void main() {
    vo_frag_pos = vec3(model * vec4(a_pos, 1.0));

    vo_normal = transpose(inverse(mat3(model))) * a_normal;

    vo_tex_coords = a_tex_coord;

    gl_Position = projection * view * vec4(vo_frag_pos, 1.0);
}

