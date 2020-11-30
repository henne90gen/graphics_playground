#version 330 core

in vec2 position;
in float height;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float h;

void main() {
    h = height;
    gl_Position = projection * view * model * vec4(position.x, height, position.y, 1.0);
}
