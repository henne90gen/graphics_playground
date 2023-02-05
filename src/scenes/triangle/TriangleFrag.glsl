#version 300 es
precision mediump float;

in vec3 passColor;
in vec3 v_Position;

out vec4 color;

vec3 map(vec3 value, vec3 inMin, vec3 inMax, vec3 outMin, vec3 outMax) {
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

void main() {
    vec3 finalColor = passColor * map(v_Position, vec3(-1.0F), vec3(1.0F), vec3(0.0F), vec3(1.0F));
    color = vec4(finalColor, 1.0F);
}
