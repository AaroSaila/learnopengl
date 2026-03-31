#version 460 core

uniform vec3 color;
uniform sampler2D texture_map;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    vec4 texture_color = texture(texture_map, tex_coords);
    frag_color = texture_color;
}
