#version 460 core

layout (location = 0) in vec3 a_pos;
// layout (location = 1) in vec3 a_normal;
// layout (location = 2) in vec2 a_tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
// uniform mat3 normal_matrix;

// out vec2 tex_coords;
// out vec3 normal_vector;
// out vec3 frag_pos;

void main() {
   gl_Position = projection * view * model * vec4(a_pos, 1.0f);
   // tex_coords = a_tex_coords;
   // normal_vector = normalize(normal_matrix * a_normal);
   // frag_pos = vec3(model * vec4(a_pos, 1.0));
}
