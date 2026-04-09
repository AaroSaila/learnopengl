#version 460 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec3 a_color;

out VS_OUT {
    vec3 color;
} vs_out;

void main() {
    gl_Position = vec4(a_pos.x, a_pos.y, 0.0, 1.0);
    vs_out.color = a_color;
}
