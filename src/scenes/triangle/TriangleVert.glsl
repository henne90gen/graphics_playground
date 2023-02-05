#version 300 es
precision mediump float;

in vec2 position;
in vec3 color;

out vec3 passColor;
out vec3 v_Position;

void main() {
    passColor = color;
    v_Position = vec3(position, 0.0F);
    gl_Position = vec4(v_Position, 1.0F);
}
