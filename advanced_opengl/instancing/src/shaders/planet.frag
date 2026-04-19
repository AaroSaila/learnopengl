#version 460 core

struct Material {
    sampler2D texture_diffuses[3];
};

uniform Material material;

in vec2 tex_coords;

out vec4 frag_color;

void main() {
    vec3 diffuse_color = vec3(texture(material.texture_diffuses[0], tex_coords));

    frag_color = vec4(diffuse_color, 1.0);
}
