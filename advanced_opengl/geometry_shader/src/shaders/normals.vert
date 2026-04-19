#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform mat4 view;
uniform mat4 model;

out VS_OUT {
    vec3 normal;
} vs_out;

void main() {
    gl_Position = view * model * vec4(a_pos, 1.0);
    mat3 normal_matrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = normalize(vec3(vec4(normal_matrix * a_normal, 0.0)));
}
