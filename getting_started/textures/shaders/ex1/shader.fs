#version 460 core

in vec2 tex_coord;

out vec4 frag_color;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    frag_color = mix(texture(texture1, tex_coord), texture(texture2, vec2(-tex_coord.x, tex_coord.y)), 0.2);
}
