#version 330

in float vHeight;

uniform float maxHeight;

out vec4 color;

void main() {
    color = vec4(0.0, vHeight / maxHeight, 0.0, 1.0);
}
