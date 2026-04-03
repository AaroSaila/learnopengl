#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normal_vector;
out vec3 position;

void main() {
   normal_vector = mat3(transpose(inverse(model))) * a_normal;
   position = vec3(model * vec4(a_pos, 1.0));
   gl_Position = projection * view * vec4(position, 1.0);
}
