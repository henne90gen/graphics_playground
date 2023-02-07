#version 300 es
precision mediump float;

in vec3 a_Position;
in vec2 a_UV;

out vec2 coords;

void main() {
    gl_Position = vec4(a_Position, 1.0);
    coords = a_UV;
}