#version 460 core

uniform sampler2D diffuse_texture;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform bool blinn_enable;

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
    vec3 view_dir = normalize(view_pos - frag_pos);
    float spec_exponent = 8.0;
    if (blinn_enable) {
        vec3 halfway = normalize(light_dir + view_dir);
        float mag = pow(max(dot(normal_vector, halfway), 0.0), spec_exponent);
        rgb += vec3(0.4) * mag;
    } else {
        vec3 reflect_dir = normalize(reflect(-light_dir, normal_vector));
        float mag = pow(max(dot(view_dir, reflect_dir), 0.0), spec_exponent);
        rgb += vec3(0.4) * mag;
    }

    frag_color = vec4(rgb, 1.0);
}
