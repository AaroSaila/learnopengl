#version 460 core

uniform sampler2D depth_map;
uniform bool is_perspective;
uniform float near_plane;
uniform float far_plane;

in vec2 tex_coords;

out vec4 frag_color;

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main() {
    float depth_value = texture(depth_map, tex_coords).r;
    if (is_perspective) {
        frag_color = vec4(vec3(linearize_depth(depth_value) / far_plane), 1.0);
        // frag_color = vec4(vec3(depth_value), 1.0);
    } else {
        frag_color = vec4(vec3(depth_value), 1.0);
    }
}
