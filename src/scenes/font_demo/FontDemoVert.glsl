#version 330 core

in vec2 position;
in vec2 vertexUV;

uniform mat4 model;
uniform mat4 view;

out vec2 UV;

void main() {
    UV = vertexUV;
    gl_Position = view * model * vec4(position, 0.0, 1.0);
}
