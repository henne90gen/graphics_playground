#version 300 es
precision mediump float;

in vec2 position;
in vec2 vertexUV;

out vec2 UV;

void main() {
    UV = vertexUV;
    gl_Position = vec4(position, 0.0F, 1.0F);
}
