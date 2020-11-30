#version 330 core

in vec3 passColor;
in vec3 v_Position;

out vec4 color;

vec3 map(vec3 value, vec3 inMin, vec3 inMax, vec3 outMin, vec3 outMax) {
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

void main() {
    vec3 finalColor = passColor * map(v_Position, vec3(-1), vec3(1), vec3(0), vec3(1));
    color = vec4(finalColor, 1.0);
}
