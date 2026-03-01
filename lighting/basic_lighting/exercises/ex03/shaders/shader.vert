#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos;

out vec3 frag_pos;
out vec3 normal;
out vec3 _light_pos;

void main() {
    mat4 view_model = view * model;
    gl_Position = projection * view_model * vec4(a_pos, 1.0f);
    frag_pos = vec3(view_model * vec4(a_pos, 1.0));
    normal = mat3(transpose(inverse(view_model))) * a_normal;
    _light_pos = vec3(view_model * vec4(light_pos, 1.0));
}
