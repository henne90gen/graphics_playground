#version 330 core

in vec3 a_Position;
in vec2 a_UV;

uniform mat4 modelMatrix;

out vec2 UV;

void main() {
    UV = a_UV;
    gl_Position = modelMatrix * vec4(a_Position, 1.0);
}
