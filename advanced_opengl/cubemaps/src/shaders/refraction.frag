#version 460 core

uniform samplerCube skybox;
uniform vec3 camera_position;

in vec3 normal_vector;
in vec3 position;

out vec4 frag_color;

void main() {
    float ratio = 1.0 / 1.52;
    vec3 I = normalize(position - camera_position);
    vec3 R = refract(I, normalize(normal_vector), ratio);
    frag_color = vec4(texture(skybox, R).rgb, 1.0);
}
