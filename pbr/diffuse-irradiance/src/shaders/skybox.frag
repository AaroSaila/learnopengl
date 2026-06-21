#version 460 core

uniform samplerCube u_env_map;

in vec3 local_pos;

out vec4 frag_color;

void main() {
    vec3 env_color = texture(u_env_map, local_pos).rgb;

    env_color = env_color / (env_color + vec3(1.0)); // hdr
    env_color = pow(env_color, vec3(1.0 / 2.2)); // gamma correction

    frag_color = vec4(env_color, 1.0);
}
