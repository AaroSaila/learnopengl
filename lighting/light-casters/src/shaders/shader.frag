#version 460 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 direction;
    float inner_cutoff;
    float outer_cutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 frag_pos;
in vec3 normal;
in vec2 tex_coords;

out vec4 frag_color;

uniform vec3 view_pos;
uniform Material material;
uniform Light light;

void main() {
    vec3 rgb;

    vec3 diffuse_color = vec3(texture(material.diffuse, tex_coords));
    vec3 specular_intensity = vec3(texture(material.specular, tex_coords));

    vec3 ambient = diffuse_color * light.ambient;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - frag_pos);
    vec3 diffuse = diffuse_color * max(dot(norm, light_dir), 0.0) * light.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(reflect_dir, view_dir), 0.0), material.shininess);
    vec3 specular = specular_intensity * spec * light.specular;

    float light_distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * light_distance + light.quadratic * light_distance * light_distance);

    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.inner_cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    rgb = ambient + ((diffuse + specular) * attenuation * intensity);

    frag_color = vec4(rgb, 1.0);
}
