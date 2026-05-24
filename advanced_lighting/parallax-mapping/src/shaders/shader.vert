#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;
layout (location = 3) in vec3 a_tangent;
layout (location = 4) in vec3 a_bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform bool normal_map_disabled;

out VS_OUT {
    vec3 frag_pos;
    vec3 normal_vector;
    vec2 tex_coords;
    vec3 light_pos;
    vec3 view_pos;
} vs_out;

void main() {
    vs_out.frag_pos = vec3(model * vec4(a_pos, 1.0));
    vs_out.normal_vector = transpose(inverse(mat3(model))) * a_normal;
    vs_out.tex_coords = a_tex_coord;
    gl_Position = projection * view * vec4(vs_out.frag_pos, 1.0);

    if (!normal_map_disabled) {
        // vec3 T = normalize(vec3(model * vec4(a_tangent, 0.0)));
        // vec3 B = normalize(vec3(model * vec4(a_bitangent, 0.0)));
        // vec3 N = normalize(vec3(model * vec4(a_normal, 0.0)));
        // mat3 TBN = transpose(mat3(T, B, N));

        vec3 T = normalize(vec3(model * vec4(a_tangent, 0.0)));
        vec3 N = normalize(vec3(model * vec4(a_normal, 0.0)));
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);
        mat3 TBN = transpose(mat3(T, B, N));

        vs_out.light_pos = TBN * light_pos;
        vs_out.view_pos = TBN * view_pos;
        vs_out.frag_pos = TBN * vs_out.frag_pos;
    } else {
        vs_out.light_pos = light_pos;
        vs_out.view_pos = view_pos;
    }
}

