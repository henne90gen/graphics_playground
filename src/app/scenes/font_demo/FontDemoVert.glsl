#version 130

attribute vec2 position;
attribute vec2 vertexUV;

uniform mat4 model;
uniform mat4 view;

varying vec2 UV;

void main() {
    UV = vertexUV;
    gl_Position = view * model * vec4(position, 0.0, 1.0);
}
