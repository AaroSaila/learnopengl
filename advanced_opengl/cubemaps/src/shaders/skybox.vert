#version 460 core

layout (location = 0) in vec3 a_pos;

uniform mat4 projection;
uniform mat4 view;

out vec3 tex_coords;

void main() {
    vec4 pos = projection * view * vec4(a_pos, 1.0);

    gl_Position = pos.xyww;
    tex_coords = a_pos;
}
