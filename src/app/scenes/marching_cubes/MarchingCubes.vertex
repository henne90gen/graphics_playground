#version 130

attribute vec3 position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 offset;

varying vec3 finalPosition;

void main() {
    finalPosition = position + offset;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(finalPosition, 1.0);
}
