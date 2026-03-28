#version 460 core

uniform vec3 color;

out vec4 frag_color;
out float trash;

float near = 0.1;
float far = 100.0;

float linearize_depth(float depth) {
    float ndc = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndc * (far - near));
}

void main() {
    // frag_color = vec4(color, 1.0);

    float depth = linearize_depth(gl_FragCoord.z) / far;
    frag_color = vec4(vec3(depth), 1.0);

    trash = color.r + color.g + color.b;
}
