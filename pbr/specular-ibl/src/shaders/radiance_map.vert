#version 460 core

uniform mat4 u_projection;
uniform mat4 u_view;

layout (location = 0) in vec3 a_pos;

out vec3 local_pos;

void main() {
    local_pos = a_pos;
    gl_Position = u_projection * u_view * vec4(local_pos, 1.0);
}
