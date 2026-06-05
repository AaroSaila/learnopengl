#version 460 core

uniform sampler2D hdr_buf;
uniform sampler2D bloom_buf;
uniform bool hdr_disabled;
uniform bool bloom_disabled;
uniform float exposure;

in vec2 tex_coord;

out vec4 frag_color;

void main() {
    vec3 rgb = texture(hdr_buf, tex_coord).rgb;

    if (!bloom_disabled) {
        rgb += texture(bloom_buf, tex_coord).rgb;
    }
    if (!hdr_disabled) {
        // Reinhard tone-mapping
        // rgb = rgb / (rgb + vec3(1.0));

        // exposure
        rgb = vec3(1.0) - exp(-rgb * exposure);
    }

    rgb = pow(rgb, vec3(1.0 / 2.2));

    frag_color = vec4(rgb, 1.0);
}
