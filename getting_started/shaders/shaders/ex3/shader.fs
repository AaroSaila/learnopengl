#version 460 core

in vec3 vertex_pos;

out vec4 frag_color;

void main() {
    frag_color = vec4(vertex_pos, 1.0);
}
