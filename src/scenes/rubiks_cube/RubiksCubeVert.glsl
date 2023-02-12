#version 300 es
precision mediump float;

in vec3 position;
in vec2 uv;
in vec3 color;

uniform mat4 cubeMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 vUV;
out vec3 vColor;

void main() {
    vUV = uv;
    vColor = color;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * cubeMatrix * vec4(position, 1.0F);
}
