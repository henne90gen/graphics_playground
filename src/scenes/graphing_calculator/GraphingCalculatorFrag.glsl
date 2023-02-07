#version 330 core

in vec2 coords;

out vec4 color;

float my_function(float x) {
    return x*x;
}

float InvLerp(float a, float b, float v) {
    return (v - a) / (b - a);
}

void main() {
    float y = my_function(coords.x);
    float distance = abs(coords.y - y);
    float t = clamp(InvLerp(0.005F, 0.01F, distance), 0.0F, 1.0F);
    color = mix(vec4(0.0F), vec4(1.0F), t);
}
