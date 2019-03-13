#version 130

attribute vec2 position;
attribute float height;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

varying float vHeight;

void main() {
    vHeight = height;
    vec3 finalPosition = vec3(position, height);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(finalPosition, 1.0);
}
