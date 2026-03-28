#version 460 core

struct Material {
    sampler2D texture_diffuses[3];
    sampler2D texture_speculars[3];
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 tex_coords;
in vec3 normal_vector;
in vec3 frag_pos;

out vec4 frag_color;
out vec3 dummy;

uniform Material material;
uniform PointLight point_light;
uniform vec3 view_pos;

void main() {
    vec3 diffuse_color = vec3(texture(material.texture_diffuses[0], tex_coords));
    vec3 specular_intensity = vec3(texture(material.texture_speculars[0], tex_coords));

    vec3 rgb = vec3(0.0);

    // point light
    {
        vec3 light_dir = normalize(point_light.position - frag_pos);

        // ambient
        vec3 ambient = diffuse_color * point_light.ambient;

        // diffuse
        float diff_mag = max(dot(normal_vector, light_dir), 0.0);
        vec3 diffuse = point_light.diffuse * diff_mag * diffuse_color;

        // specular
        vec3 reflect_dir = reflect(-light_dir, normal_vector);
        vec3 view_dir = normalize(view_pos - frag_pos);
        vec3 specular = specular_intensity * pow(max(dot(reflect_dir, view_dir), 0.0), 32) * point_light.specular;

        // attenuation
        float light_distance = length(point_light.position - frag_pos);
        float attenuation = 1.0 / (
                point_light.constant
                    + point_light.linear * light_distance
                    + point_light.quadratic * light_distance * light_distance
                );

        rgb += (ambient + diffuse + specular) * attenuation;
    }

    frag_color = vec4(rgb, 1.0);

    dummy = specular_intensity * 0.00001;
}
