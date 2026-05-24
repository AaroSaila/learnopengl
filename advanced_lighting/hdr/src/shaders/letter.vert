#version 460 core

layout (location = 0) in vec2 a_pos;

uniform mat4 model;

void main() {
    gl_Position = model * vec4(a_pos, 0.0, 1.0);
}
