#version 330

in float vHeight;

out vec4 color;

void main() {
    color = vec4(0.0, vHeight + 0.25F, 0.0, 1.0);
}
