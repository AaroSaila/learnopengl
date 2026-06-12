#version 460 core

uniform sampler2D color_buf;

in vec2 vo_tex_coords;

out vec4 frag_color;

void main() { 
    frag_color = vec4(vec3(texture(color_buf, vo_tex_coords).r), 1.0);
    // frag_color = vec4(texture(color_buf, vo_tex_coords).rgb, 1.0);
}
