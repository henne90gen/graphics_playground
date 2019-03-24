#version 130

attribute vec3 position;
attribute vec3 color;

uniform mat4 cubeMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

varying vec3 vColor;

void main() {
    vColor = color;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * cubeMatrix * vec4(position, 1.0);
}
