#version 130

attribute vec3 a_Position;
attribute vec2 a_UV;
attribute vec3 a_Normal;

varying vec2 v_UV;

void main() {
    v_UV = a_UV;
    gl_Position = vec4(a_Position, 1.0);
}
