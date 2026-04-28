#version 460 core

uniform sampler2D diffuse_texture;
uniform samplerCube depth_map;
uniform sampler2D normal_map;
uniform bool normal_map_disabled;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform float shininess;
uniform float far_plane;

in VS_OUT {
    vec3 frag_pos;
    vec3 normal_vector;
    vec2 tex_coords;
} fs_in;

out vec4 frag_color;

vec3 sample_offset_directions[20] = vec3[](
        vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
        vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
        vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
        vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
    );

float shadow_calculation(vec3 frag_pos) {
    float shadow = 0.0;
    float bias = 0.05;
    int samples = 20;
    float view_distance = length(view_pos - frag_pos);
    float disk_radius = (1.0 + (view_distance / far_plane)) / 70.0;
    vec3 light_to_frag = frag_pos - light_pos;

    for (int i = 0; i < samples; i++) {
        float closest_depth = texture(depth_map, light_to_frag + sample_offset_directions[i] * disk_radius).r;
        closest_depth *= far_plane;
        if (length(light_to_frag) - bias > closest_depth) {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);

    return shadow;
}

void main() {
    float brightness = 0.4;
    vec3 light_color = vec3(1.0);
    float ambient_mag = 0.01;
    float diffuse_mag = 1.0;
    float specular_mag = 2.0;

    vec3 rgb = texture(diffuse_texture, fs_in.tex_coords).rgb;
    vec3 normal = normalize(fs_in.normal_vector);
    if (!normal_map_disabled) {
        normal = texture(normal_map, fs_in.tex_coords).rgb;
        normal = normalize(normal * 2.0 - 1.0);
    }

    // Ambient
    vec3 ambient = light_color * ambient_mag;

    // Diffuse
    vec3 light_dir = normalize(light_pos - fs_in.frag_pos);
    float diff_strength = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff_strength * light_color * diffuse_mag;

    // Specular
    vec3 view_dir = normalize(view_pos - fs_in.frag_pos);
    vec3 halfway = normalize(view_dir + light_dir);
    float spec_mag = pow(max(dot(normal, halfway), 0.0), shininess);
    vec3 specular = spec_mag * light_color * specular_mag;

    float shadow = shadow_calculation(fs_in.frag_pos);
    float attenuation = 1.0 / pow(length(light_pos - fs_in.frag_pos), 2);

    rgb *= ambient + (1.0 - shadow) * attenuation * brightness * (diffuse + specular);

    // rgb = sample_offset_directions[8] + (rgb * far_plane * 0.0000000000000000001);

    frag_color = vec4(rgb, 1.0);
}
