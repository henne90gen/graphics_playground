#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_UV;

uniform mat4 modelMatrix;

out vec2 TexCoords;

void main() {
    TexCoords = a_UV;
    gl_Position = modelMatrix * vec4(a_Position, 1.0);
}
