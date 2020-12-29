#version 330 core

in vec3 a_Position;
in vec3 a_Normal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_Position, 1.0F);
}
