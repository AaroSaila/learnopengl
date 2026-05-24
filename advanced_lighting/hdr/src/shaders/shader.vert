#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool invert_normals;

out VS_OUT {
    vec3 frag_pos;
    vec3 normal_vector;
    vec2 tex_coords;
} vs_out;

void main() {
    vs_out.frag_pos = vec3(model * vec4(a_pos, 1.0));

    vec3 normal = invert_normals ? -a_normal : a_normal;
    vs_out.normal_vector = transpose(inverse(mat3(model))) * normal;

    vs_out.tex_coords = a_tex_coord;

    gl_Position = projection * view * vec4(vs_out.frag_pos, 1.0);
}

