#version 460 core

uniform sampler2D diffuse_texture;
uniform sampler2D shadow_map;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform float shininess;

in VS_OUT {
    vec3 frag_pos;
    vec3 normal_vector;
    vec2 tex_coords;
    vec4 frag_pos_light_space;
} fs_in;

out vec4 frag_color;

float shadow_calculation(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir) {
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;

    if (proj_coords.z > 1.0) {
        return 0.0;
    }

    proj_coords = proj_coords * 0.5 + 0.5;

    // float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);
    float bias = 0.005;
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
            shadow += (proj_coords.z - bias) > pcf_depth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    return shadow;
}

void main() {
    vec3 rgb = texture(diffuse_texture, fs_in.tex_coords).rgb;
    vec3 normal = normalize(fs_in.normal_vector);
    vec3 light_color = vec3(1.0);

    // Ambient
    vec3 ambient = 0.05 * light_color;

    // Diffuse
    vec3 light_dir = normalize(light_pos - fs_in.frag_pos);
    float diff_mag = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff_mag * light_color * 20.0;

    // Specular
    vec3 view_dir = normalize(view_pos - fs_in.frag_pos);
    vec3 halfway = normalize(view_dir + light_dir);
    float spec_mag = pow(max(dot(normal, halfway), 0.0), shininess);
    vec3 specular = spec_mag * light_color * 10.5;

    // Shadow
    float shadow = shadow_calculation(fs_in.frag_pos_light_space, normal, light_dir);

    float attenuation = 1.0 / pow(length(light_pos - fs_in.frag_pos), 2);

    rgb *= ambient + (1.0 - shadow) * attenuation * (diffuse + specular);

    frag_color = vec4(rgb, 1.0);
}
