#version 460 core

uniform float time;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 tex_coords;
} gs_in[];

out vec2 tex_coords;

vec3 get_normal() {
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal) {
    float magnitude = 4.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
    return position + vec4(direction, 0.0);
}

void main() {
    vec3 normal = get_normal();

    for (uint i = 0; i < 3; i++) {
        // Explode
        // gl_Position = explode(gl_in[i].gl_Position, normal);
        // tex_coords = gs_in[i].tex_coords;
        // EmitVertex();

        // Passthrough
        gl_Position = gl_in[i].gl_Position;
        tex_coords = gs_in[i].tex_coords;
        EmitVertex();
    }

    EndPrimitive();
}
