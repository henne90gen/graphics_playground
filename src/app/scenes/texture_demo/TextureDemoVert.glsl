#version 330 core

in vec2 position;
in vec2 vertexUV;

out vec2 UV;

void main() {
    UV = vertexUV;
    gl_Position = vec4(position, 0.0, 1.0);
}
