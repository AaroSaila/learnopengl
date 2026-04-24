#version 460 core

uniform sampler2D screen_texture;

in vec2 tex_coords;

out vec4 frag_color;

const float offset = 1.0 / 300.0;
const vec2 offsets[9] = vec2[](
        vec2(-offset, offset), // top-left
        vec2(0.0f, offset), // top-center
        vec2(offset, offset), // top-right
        vec2(-offset, 0.0f), // center-left
        vec2(0.0f, 0.0f), // center-center
        vec2(offset, 0.0f), // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f, -offset), // bottom-center
        vec2(offset, -offset) // bottom-right;
    );

const float kernel_sharpen[9] = float[](
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1
    );
const float kernel_blur[9] = float[](
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
        2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
    );
const float kernel_edge_detection[9] = float[](
        1, 1, 1,
        1, -8, 1,
        1, 1, 1
    );

vec4 inversion(vec3 rgb) {
    return vec4(1.0 - rgb, 1.0);
}

vec4 grayscale(vec3 rgb) {
    float average = (rgb.r * 0.2126 + rgb.g * 0.7152 + rgb.b * 0.0722) / 3.0;
    return vec4(average, average, average, 1.0);
}

vec4 apply_kernel(float kernel[9]) {
    vec3 rgb = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        vec3 sample_tex = vec3(texture(screen_texture, tex_coords.st + offsets[i]));
        rgb += sample_tex * kernel[i];
    }

    return vec4(rgb, 1.0);
}

void main() {
    vec4 texture_color = texture(screen_texture, tex_coords);
    frag_color = texture_color;
    // frag_color = grayscale(texture_color.rgb);
    // frag_color = apply_kernel(kernel_edge_detection);
}
