#version 460 core

uniform sampler2D hdr_buf;
uniform bool hdr_disabled;
uniform float exposure;

in vec2 tex_coord;

out vec4 frag_color;

void main() {
    vec3 rgb = texture(hdr_buf, tex_coord).rgb;
    
    if (!hdr_disabled) {
        // Reinhard tone-mapping
        // rgb = rgb / (rgb + vec3(1.0));

        // exposure
        rgb = vec3(1.0) - exp(-rgb * exposure);
    }

    frag_color = vec4(rgb, 1.0);
}
