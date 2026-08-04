#version 460 core

uniform samplerCube env_map;
uniform float roughness;

in vec3 local_pos;

out vec4 frag_color;

const float PI = 3.14159265359;

float radical_inverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), radical_inverse_VdC(i));
}

vec3 importance_sample_ggx(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    // spherical coordinates to cartesian
    vec3 H;
    H.x = cos(phi) * sin_theta;
    H.y = sin(phi) * sin_theta;
    H.z = cos_theta;

    // tangent-space to world-space
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sample_vec = tangent * H.x + bitangent * H.y + N * H.z;

    return normalize(sample_vec);
}

float distribution_ggx(float NdotH, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return num / denom;
}

void main() {
    vec3 N = normalize(local_pos);
    vec3 R = N;
    vec3 V = R;

    const uint sample_count = 1024u;
    float total_weight = 0.0;
    vec3 prefiltered_color = vec3(0.0);
    for (uint i = 0u; i < sample_count; i++) {
        vec2 Xi = hammersley(i, sample_count);
        vec3 H = importance_sample_ggx(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float D = distribution_ggx(NdotH, roughness);
        float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;

        float resolution = 512.0; // hdr map res
        float sa_texel = 4.0 * PI / (6.0 * resolution * resolution);
        float sa_sample = 1.0 / (float(sample_count) * pdf + 0.0001);

        float mip_level = roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample / sa_texel);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            prefiltered_color += textureLod(env_map, L, mip_level).rgb * NdotL;
            total_weight += NdotL;
        }
    }

    prefiltered_color = prefiltered_color / total_weight;

    frag_color = vec4(prefiltered_color, 1.0);
}
