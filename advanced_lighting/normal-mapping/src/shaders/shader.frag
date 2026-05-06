#version 460 core

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform bool normal_map_disabled;
uniform float shininess;

in VS_OUT {
    vec3 frag_pos;
    vec3 normal_vector;
    vec2 tex_coords;
    vec3 light_pos;
    vec3 view_pos;
} fs_in;

out vec4 frag_color;

void main() {
    float brightness = 1.0;
    vec3 light_color = vec3(1.0);
    float ambient_mag = 0.01;
    float diffuse_mag = 1.0;
    float specular_mag = 4.0;

    vec3 rgb = texture(diffuse_map, fs_in.tex_coords).rgb;
    vec3 normal = normalize(fs_in.normal_vector);
    if (!normal_map_disabled) {
        normal = texture(normal_map, fs_in.tex_coords).rgb;
        normal = normalize(normal * 2.0 - 1.0);
    }

    // Ambient
    vec3 ambient = light_color * ambient_mag;

    // Diffuse
    vec3 light_dir = normalize(fs_in.light_pos - fs_in.frag_pos);
    float diff_strength = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff_strength * light_color * diffuse_mag;

    // Specular
    vec3 view_dir = normalize(fs_in.view_pos - fs_in.frag_pos);
    vec3 halfway = normalize(view_dir + light_dir);
    float spec_mag = pow(max(dot(normal, halfway), 0.0), shininess);
    vec3 specular = spec_mag * light_color * specular_mag;

    float attenuation = 1.0 / pow(length(fs_in.light_pos - fs_in.frag_pos), 2);

    rgb *= ambient + (attenuation * brightness * (diffuse + specular));

    frag_color = vec4(rgb, 1.0);
}
