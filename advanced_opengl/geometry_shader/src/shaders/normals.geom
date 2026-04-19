#version 460 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

uniform mat4 projection;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.4;

void main() {
    for (uint i = 0; i < 3; i++) {
        gl_Position = projection * gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = projection * (
                gl_in[i].gl_Position + vec4(gs_in[i].normal, 0.0) * MAGNITUDE
                );
        EmitVertex();
        EndPrimitive();
    }
}
