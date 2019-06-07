#version 130

attribute vec2 position;
attribute vec2 vertexUV;

uniform mat4 model;

varying vec2 UV;

void main() {
    UV = vertexUV;
    gl_Position = model * vec4(position, 0.0, 1.0);
}
