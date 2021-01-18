#version 330 core

in vec3 position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelmatrix * vec4(position, 1.0);
}
