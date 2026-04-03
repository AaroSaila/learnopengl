#version 460 core

uniform vec3 camera_pos;
uniform samplerCube skybox;

in vec3 normal_vector;
in vec3 position;

out vec4 frag_color;

void main() {
    vec3 I = normalize(position - camera_pos);
    vec3 R = reflect(I, normalize(normal_vector));
    frag_color = vec4(texture(skybox, R).rgb, 1.0);
}
