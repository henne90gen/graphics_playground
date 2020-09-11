#version 330 core

in vec3 oColor;

out vec4 color;

void main() {
    color = vec4(oColor, 1.0);
    color = vec4(1.0, 1.0, 1.0, 1.0);
}
