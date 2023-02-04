#version 300 es
precision mediump float;

uniform vec3 u_Color;

out vec4 color;

void main() {
    color = vec4(u_Color, 1.0F);
}
