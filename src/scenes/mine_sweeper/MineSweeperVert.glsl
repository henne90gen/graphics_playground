#version 300 es
precision mediump float;

in vec3 a_Position;
in vec2 a_UV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_Position, 1.0F);
    UV = a_UV;
}