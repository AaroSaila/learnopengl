#version 460 core

uniform sampler2D diffuse_map;
uniform bool color;
uniform bool specular;

in vec2 vo_tex_coords;

out vec4 frag_color;

void main() {
    if (color) {
        frag_color = vec4(texture(diffuse_map, vo_tex_coords).rgb, 1.0);
    } else if (specular) {
        frag_color = vec4(vec3(texture(diffuse_map, vo_tex_coords).a), 1.0);
    } else {
        frag_color = texture(diffuse_map, vo_tex_coords);
    }
}
