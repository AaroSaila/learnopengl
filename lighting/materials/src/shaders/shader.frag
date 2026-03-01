#version 460 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_color;

uniform vec3 view_pos;
uniform Material material;
uniform Light light;

void main() {
    vec3 ambient = material.ambient * light.ambient;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * material.diffuse * light.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(reflect_dir, view_dir), 0.0), material.shininess);
    vec3 specular = material.specular * spec * light.specular;

    vec3 rgb = ambient + diffuse + specular;
    frag_color = vec4(rgb, 1.0);
}
