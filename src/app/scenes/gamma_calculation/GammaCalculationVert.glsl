#version 330 core

in vec2 position;
in vec2 vertexUV;

uniform mat4 modelMatrix;

out vec2 UV;

void main() {
    UV = vertexUV;
    gl_Position = modelMatrix * vec4(position, 0.0, 1.0);
}
