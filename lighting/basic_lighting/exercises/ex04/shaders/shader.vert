#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

out vec4 color;

void main() {
    gl_Position = projection * view * model * vec4(a_pos, 1.0f);

    vec3 frag_pos = vec3(model * vec4(a_pos, 1.0));
    vec3 normal = mat3(transpose(inverse(model))) * a_normal;

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

    color = vec4((ambient + diffuse + specular) * object_color, 1.0);
}
