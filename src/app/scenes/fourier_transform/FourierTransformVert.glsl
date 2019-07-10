#version 130

attribute vec2 a_Position;
attribute vec2 a_UV;

varying vec2 v_UV;

uniform bool u_RenderCanvas;

void main() {
    if (u_RenderCanvas) {
        v_UV = a_UV;
    }
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
