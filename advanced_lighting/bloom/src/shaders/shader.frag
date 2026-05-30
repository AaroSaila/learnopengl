#version 460 core

struct point_light {
    vec3 pos;
    vec3 color;
};

uniform sampler2D diffuse_map;
uniform float shininess;
uniform vec3 view_pos;

#define LIGHTS_COUNT 4
uniform point_light point_lights[LIGHTS_COUNT];

in VS_OUT {
    vec3 frag_pos;
    vec3 normal_vector;
    vec2 tex_coords;
} fs_in;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;

void main() {
    float ambient_mag = 0.00;
    float diffuse_mag = 1.0;
    float specular_mag = 0.5;

    vec3 view_dir = normalize(view_pos - fs_in.frag_pos);
    vec2 tex_coords = fs_in.tex_coords;

    vec3 normal = normalize(fs_in.normal_vector);
    vec3 rgb = texture(diffuse_map, tex_coords).rgb;

    vec3 lighting = vec3(0.0);
    for (int i = 0; i < LIGHTS_COUNT; i++) {
        // Ambient
        vec3 ambient = point_lights[i].color * ambient_mag;

        // Diffuse
        vec3 light_dir = normalize(point_lights[i].pos - fs_in.frag_pos);
        float diff_strength = max(dot(light_dir, normal), 0.0);
        vec3 diffuse = diff_strength * point_lights[i].color * diffuse_mag;

        // Specular
        vec3 halfway = normalize(view_dir + light_dir);
        float spec_mag = pow(max(dot(normal, halfway), 0.0), shininess);
        vec3 specular = spec_mag * point_lights[i].color * specular_mag;

        float attenuation = 1.0 / pow(length(point_lights[i].pos - fs_in.frag_pos), 2);

        lighting += ambient + (attenuation * (diffuse + specular));
    }

    frag_color = vec4(rgb * lighting, 1.0);

    float brightness = dot(frag_color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0) {
        bright_color = vec4(frag_color.rgb, 1.0);
    } else {
        bright_color = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
