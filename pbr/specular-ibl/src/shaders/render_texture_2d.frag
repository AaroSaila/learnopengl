#version 460 core

uniform sampler2D tex;

in vec2 vo_tex_coords;

out vec4 frag_color;

void main() {
    vec4 tex_sample = texture(tex, vo_tex_coords);
    // vec3 color = vec3(tex_sample.x);
    frag_color = tex_sample;
}
