#version 460 core

#define LIGHTS_COUNT 4

uniform vec3 u_camera_pos;
// uniform sampler2D u_albedo_map;
// uniform sampler2D u_normal_map;
// uniform sampler2D u_metallic_map;
// uniform sampler2D u_roughness_map;
// uniform sampler2D u_ao_map;
// uniform samplerCube u_irradiance_map;
uniform vec3 u_albedo;
uniform float u_normal;
uniform float u_metallic;
uniform float u_roughness;
uniform float u_ao;
uniform samplerCube u_irradiance_map;
uniform vec3[LIGHTS_COUNT] u_light_positions;
uniform vec3[LIGHTS_COUNT] u_light_colors;
uniform uint u_lights_count;
uniform bool u_use_irradiance_map;

in vec2 tex_coords;
in vec3 world_pos;
in vec3 normal;

out vec4 frag_color;

const float PI = 3.14159265359;

// Fresnel
vec3 fresnel_schlick(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

vec3 fresnel_schlick_roughness(float cos_theta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

// Distribution
float distribution_ggx(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return num / denom;
}

// Geometry
float geometry_schlick_ggx(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);
    float ggx1 = geometry_schlick_ggx(NdotL, roughness);
    return ggx1 * ggx2;
}

// From demo code
// vec3 getNormalFromMap()
// {
//     vec3 tangentNormal = texture(u_normal_map, tex_coords).xyz * 2.0 - 1.0;
//
//     vec3 Q1  = dFdx(world_pos);
//     vec3 Q2  = dFdy(world_pos);
//     vec2 st1 = dFdx(tex_coords);
//     vec2 st2 = dFdy(tex_coords);
//
//     vec3 N   = normalize(normal);
//     vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
//     vec3 B  = -normalize(cross(N, T));
//     mat3 TBN = mat3(T, B, N);
//
//     return normalize(TBN * tangentNormal);
// }

void main() {
    // vec3 albedo = pow(texture(u_albedo_map, tex_coords).rgb, vec3(2.2));
    // vec3 normal = getNormalFromMap();
    // float metallic = texture(u_metallic_map, tex_coords).r;
    // float roughness = texture(u_roughness_map, tex_coords).r;
    // float ao = texture(u_ao_map, tex_coords).r;
    vec3 albedo = u_albedo;
    float metallic = u_metallic;
    float roughness = u_roughness;
    float ao = u_ao;

    vec3 N = normalize(normal);
    vec3 V = normalize(u_camera_pos - world_pos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < u_lights_count; i++) {
        vec3 L = normalize(u_light_positions[i] - world_pos);
        vec3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);

        float distance = length(u_light_positions[i] - world_pos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = u_light_colors[i] * attenuation;

        vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);
        
        float NDF = distribution_ggx(N, H, roughness);
        float G = geometry_smith(N, V, L, roughness);

        vec3 num = NDF * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        vec3 specular = num / denom;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    if (u_use_irradiance_map) {
        vec3 kS = fresnel_schlick_roughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 kD = 1.0 - kS;
        vec3 irradiance = texture(u_irradiance_map, N).rgb;
        vec3 diffuse = irradiance * albedo;
        ambient = kD * diffuse * ao;
    }

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    frag_color = vec4(color, 1.0);
}

