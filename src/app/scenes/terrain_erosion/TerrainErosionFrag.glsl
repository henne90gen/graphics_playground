#version 330

in float vHeight;

uniform float maxHeight;

out vec4 color;

void main() {
    float heightScale = vHeight / maxHeight;
    color = vec4(0.0F, heightScale, 0.0F, 1.0F);
}
