#version 460 core

uniform samplerCube skybox;

in vec3 tex_coords;

out vec4 frag_color;

void main() {
    frag_color = texture(skybox, tex_coords);
}
