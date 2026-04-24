#version 460 core

uniform sampler2D diffuse_texture;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform bool gamma_enabled;

in vec3 frag_pos;
in vec2 tex_coords;
in vec3 normal_vector;

out vec4 frag_color;

void main() {
    vec3 rgb = vec3(0.0);

    vec3 diffuse_color = vec3(texture(diffuse_texture, tex_coords));
    vec3 light_dir = normalize(light_pos - frag_pos);

    // Ambient
    {
        float mag = 0.1f;
        rgb += diffuse_color * mag;
    }

    // Diffuse
    {
        float mag = max(dot(normal_vector, light_dir), 0.0);
        rgb += diffuse_color * mag;
    }

    // Specular
    {
        vec3 view_dir = normalize(view_pos - frag_pos);
        vec3 halfway = normalize(light_dir + view_dir);
        float mag = pow(max(dot(normal_vector, halfway), 0.0), 9.0);
        rgb += vec3(0.4) * mag;
    }

    if (gamma_enabled) {
        float attenuation = 1.0 / pow(length(light_pos - frag_pos), 2);
        rgb *= attenuation;
    } else {
        float attenuation = 1.0 / length(light_pos - frag_pos);
        rgb *= attenuation;
    }

    rgb = pow(rgb, vec3(1.0 / 2.2));

    frag_color = vec4(rgb, 1.0);
}
