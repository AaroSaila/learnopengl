#version 460 core

uniform sampler2D diffuse_map;
uniform sampler2D specular_map;

in vec2 vo_tex_coords;
in vec3 vo_frag_pos;
in vec3 vo_normal;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_color_spec;

void main() {
    g_position = vo_frag_pos;
    g_normal = normalize(vo_normal);
    g_color_spec.rgb = texture(diffuse_map, vo_tex_coords).rgb;
    g_color_spec.a = texture(specular_map, vo_tex_coords).r;
}
