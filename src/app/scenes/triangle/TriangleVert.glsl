#version 130

attribute vec2 position;
attribute vec3 color;

varying vec3 passColor;
varying vec3 v_Position;

void main() {
    passColor = color;
    v_Position = vec3(position, 0);
    gl_Position = vec4(v_Position, 1.0);
}
