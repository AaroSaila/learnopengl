#version 460 core

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D noise_map;
uniform vec2 noise_scale;
#define SAMPLE_COUNT 64
uniform vec3 samples[SAMPLE_COUNT];
uniform mat4 view;
uniform mat4 projection;

in vec2 vo_tex_coords;

out float frag_color;

void main() {
    vec3 frag_pos = texture(g_position, vo_tex_coords).xyz;
    vec3 normal = texture(g_normal, vo_tex_coords).xyz;
    vec3 random_vec = texture(noise_map, vo_tex_coords * noise_scale).xyz;

    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    const int kernel_size = SAMPLE_COUNT;
    const float radius = 0.5 * 4;
    const float bias = 0.025;
    for (int i = 0; i < kernel_size; i++) {
        vec3 sample_pos = tbn * samples[i];
        sample_pos = frag_pos + sample_pos * radius;

        vec4 offset = vec4(sample_pos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sample_depth = texture(g_position, offset.xy).z;
        // occlusion += sample_depth >= sample_pos.z + bias ? 1.0 : 0.0;
        float range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
    }

    occlusion = 1.0 - (occlusion / kernel_size);

    frag_color = pow(occlusion, 2);
}
