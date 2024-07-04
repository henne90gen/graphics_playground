#version 300 es
precision mediump float;

in vec3 a_Position;
in vec2 a_UV;

uniform mat4 u_ViewMatrix;

out vec2 v_UV;

void main() {
    v_UV = a_UV;
    gl_Position = u_ViewMatrix * vec4(a_Position, 1.0);
}
