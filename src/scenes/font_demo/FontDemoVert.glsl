#version 300 es
precision mediump float;

in vec2 position;
in vec2 vertexUV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 UV;

void main() {
    UV = vertexUV;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 0.0F, 1.0F);
}
