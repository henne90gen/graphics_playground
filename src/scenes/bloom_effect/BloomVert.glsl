#version 300 es
precision mediump float;

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_UV;

out vec2 UV;

uniform mat4 u_ModelMatrix;

void main() {
    UV = a_UV;
    gl_Position = u_ModelMatrix * vec4(a_Position, 1.0);
}
