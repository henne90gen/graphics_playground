#version 330 core

in vec2 position;
in vec2 inUV;

uniform mat4 projectionMatrix;

out vec2 UV;

void main() {
    UV = inUV;
    gl_Position = projectionMatrix * vec4(position, 0.0F, 1.0F);
}
