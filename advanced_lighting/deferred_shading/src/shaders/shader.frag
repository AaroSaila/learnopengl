#version 460 core

struct PointLight {
    vec3 pos;
    vec3 color;
};

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_color_specular;
uniform vec3 view_pos;
uniform float exposure;
uniform bool hdr_disabled;

#define LIGHTS_COUNT 27
uniform PointLight point_lights[LIGHTS_COUNT];

in vec2 vo_tex_coords;

out vec4 frag_color;

void main() {
    vec3 frag_pos = texture(g_position, vo_tex_coords).xyz;
    vec3 normal = normalize(texture(g_normal, vo_tex_coords).rgb);
    vec3 rgb = texture(g_color_specular, vo_tex_coords).rgb;
    float specular_material = texture(g_color_specular, vo_tex_coords).a;

    float ambient_mag = 0.0;
    float diffuse_mag = 0.3;
    float specular_mag = 1.0;

    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 lighting = vec3(0.0);
    for (int i = 0; i < LIGHTS_COUNT; i++) {
        // Ambient
        vec3 ambient = point_lights[i].color * ambient_mag;

        // Diffuse
        vec3 light_dir = normalize(point_lights[i].pos - frag_pos);
        float diff_strength = max(dot(light_dir, normal), 0.0);
        vec3 diffuse = diff_strength * point_lights[i].color * diffuse_mag;

        // Specular
        vec3 halfway = normalize(view_dir + light_dir);
        float spec_mag = pow(max(dot(normal, halfway), 0.0), 16.0);
        vec3 specular = spec_mag * point_lights[i].color * specular_mag * specular_material;

        float attenuation = 1.0 / pow(length(point_lights[i].pos - frag_pos), 2);

        lighting += ambient + (attenuation * (diffuse + specular));
    }

    rgb = rgb * lighting;

    if (!hdr_disabled) {
        rgb = vec3(1.0) - exp(-rgb * exposure);
    }

    rgb = pow(rgb, vec3(1.0 / 2.2));

    frag_color = vec4(rgb, 1.0);
}
