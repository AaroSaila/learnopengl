#version 460 core

uniform mat4 u_projection;
uniform mat4 u_view;

layout (location = 0) in vec3 a_pos;

out vec3 local_pos;

void main() {
    local_pos = a_pos;

    mat4 rot_view = mat4(mat3(u_view)); // Remove translation from view
    vec4 clip_pos = u_projection * rot_view * vec4(local_pos, 1.0);

    gl_Position = clip_pos.xyww;
}
