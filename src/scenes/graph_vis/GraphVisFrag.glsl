#version 300 es
precision mediump float;

in vec3 oColor;

out vec4 color;

void main() {
    color = vec4(oColor, 1.0);
}
