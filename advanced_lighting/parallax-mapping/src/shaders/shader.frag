#version 460 core

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform sampler2D depth_map;
uniform bool normal_map_disabled;
uniform bool parallax_map_disabled;
uniform float shininess;

in VS_OUT {
    vec3 frag_pos;
    vec3 normal_vector;
    vec2 tex_coords;
    vec3 light_pos;
    vec3 view_pos;
} fs_in;

out vec4 frag_color;

vec2 parallax_mapping(vec2 tex_coords, vec3 view_dir) {
    const float height_scale = 0.10;
    const float min_layers = 8.0;
    const float max_layers = 32.0;
    const float num_layers = mix(max_layers, min_layers, max(dot(vec3(0.0, 0.0, 1.0), view_dir), 0.0));
    // const float num_layers = mix(min_layers, max_layers, max(dot(vec3(0.0, 0.0, 1.0), view_dir), 0.0));

    // float height = texture(depth_map, tex_coords).r;
    // vec2 p = view_dir.xy / view_dir.z * (height * height_scale);
    // // vec2 p = view_dir.xy * (height * height_scale);
    //
    // tex_coords -= p;

    float layer_depth = 1.0 / num_layers;
    float current_layer_depth = 0.0;
    vec2 p = view_dir.xy * height_scale;
    vec2 delta_tex_coords = p / num_layers;

    vec2 current_tex_coords = tex_coords;
    float current_depth_map_value = texture(depth_map, current_tex_coords).r;

    while (current_layer_depth < current_depth_map_value) {
        current_tex_coords -= delta_tex_coords;
        current_depth_map_value = texture(depth_map, current_tex_coords).r;
        current_layer_depth += layer_depth;
    }

    if (current_tex_coords.x > 1.0 || current_tex_coords.y > 1.0 || current_tex_coords.x < 0.0 || current_tex_coords.y < 0.0) {
        discard;
    }

    vec2 prev_tex_coords = current_tex_coords + delta_tex_coords;

    float after_depth = current_depth_map_value - current_layer_depth;

    return current_tex_coords;
}

void main() {
    float brightness = 1.0;
    vec3 light_color = vec3(1.0);
    float ambient_mag = 0.01;
    float diffuse_mag = 1.0;
    float specular_mag = 4.0;

    vec3 view_dir = normalize(fs_in.view_pos - fs_in.frag_pos);
    vec2 tex_coords = fs_in.tex_coords;
    if (!parallax_map_disabled) {
        tex_coords = parallax_mapping(fs_in.tex_coords, view_dir);
    }

    vec3 rgb = texture(diffuse_map, tex_coords).rgb;
    vec3 normal = normalize(fs_in.normal_vector);
    if (!normal_map_disabled) {
        normal = texture(normal_map, tex_coords).rgb;
        normal = normalize(normal * 2.0 - 1.0);
    }

    // Ambient
    vec3 ambient = light_color * ambient_mag;

    // Diffuse
    vec3 light_dir = normalize(fs_in.light_pos - fs_in.frag_pos);
    float diff_strength = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff_strength * light_color * diffuse_mag;

    // Specular
    vec3 halfway = normalize(view_dir + light_dir);
    float spec_mag = pow(max(dot(normal, halfway), 0.0), shininess);
    vec3 specular = spec_mag * light_color * specular_mag;

    float attenuation = 1.0 / pow(length(fs_in.light_pos - fs_in.frag_pos), 2);

    rgb *= ambient + (attenuation * brightness * (diffuse + specular));

    frag_color = vec4(rgb, 1.0);
}
