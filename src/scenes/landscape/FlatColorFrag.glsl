#version 330 core

uniform vec3 flatColor;

out vec4 color;

void main() {
    color = vec4(flatColor, 1.0F);
}
