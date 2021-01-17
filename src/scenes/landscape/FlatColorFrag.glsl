#version 330 core

#include "NoiseLib.glsl"

uniform vec3 flatColor;

out vec4 color;

void main() {
    float n = gold_noise(vec2(1.0, 0.5), 1.0F);
    color = vec4(flatColor*n, 1.0F);
}
