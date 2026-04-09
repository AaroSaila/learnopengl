#version 460 core

struct Material {
    sampler2D texture_diffuses[3];
    sampler2D texture_speculars[3];
};

uniform Material material;

in vec2 tex_coords;

out vec4 frag_color;
out vec4 trash;

void main() {
    frag_color = texture(material.texture_diffuses[0], tex_coords);
    trash = texture(material.texture_speculars[0], vec2(0.0, 0.0));
    trash * 2 / 10.5;
}
