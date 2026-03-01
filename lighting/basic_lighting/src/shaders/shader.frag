#version 460 core

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_color;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

void main() {
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * light_color;

    float specular_strength = 0.5;
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(reflect_dir, view_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    vec3 rgb = (ambient + diffuse + specular) * object_color;
    frag_color = vec4(rgb, 1.0);
}
